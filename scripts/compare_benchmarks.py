#!/usr/bin/env python3

import json
import sys
import argparse
from typing import Dict, Any, List, Tuple
import statistics

def load_benchmark_results(filepath: str) -> List[Dict[str, Any]]:
    """Load benchmark results from JSON file."""
    try:
        with open(filepath, 'r') as f:
            data = json.load(f)
        
        # Handle both direct benchmark format and Google Benchmark format
        if 'benchmarks' in data:
            return data['benchmarks']
        elif isinstance(data, list):
            return data
        else:
            return [data]
            
    except FileNotFoundError:
        print(f"Error: Benchmark file '{filepath}' not found.")
        sys.exit(1)
    except json.JSONDecodeError as e:
        print(f"Error: Invalid JSON in '{filepath}': {e}")
        sys.exit(1)

def extract_benchmark_data(benchmarks: List[Dict[str, Any]]) -> Dict[str, float]:
    """Extract benchmark names and their primary metrics."""
    results = {}
    
    for bench in benchmarks:
        name = bench.get('name', 'unknown')
        
        # Try different time metric names
        time_value = None
        for time_key in ['real_time', 'cpu_time', 'wall_time', 'time', 'duration']:
            if time_key in bench:
                time_value = bench[time_key]
                break
        
        if time_value is not None:
            results[name] = float(time_value)
        else:
            print(f"Warning: No time metric found for benchmark '{name}'")
    
    return results

def compare_benchmarks(baseline: Dict[str, float], 
                      current: Dict[str, float], 
                      threshold: float = 0.05) -> Tuple[bool, List[Dict[str, Any]]]:
    """
    Compare benchmark results and detect regressions.
    
    Args:
        baseline: Baseline benchmark results (name -> time)
        current: Current benchmark results (name -> time)
        threshold: Regression threshold (e.g., 0.05 = 5% slower is regression)
    
    Returns:
        Tuple of (has_regression, detailed_results)
    """
    results = []
    has_regression = False
    
    # Find common benchmarks
    common_benchmarks = set(baseline.keys()) & set(current.keys())
    
    if not common_benchmarks:
        print("Warning: No common benchmarks found between baseline and current results")
        return False, []
    
    for name in sorted(common_benchmarks):
        baseline_time = baseline[name]
        current_time = current[name]
        
        # Calculate relative change (positive = slower, negative = faster)
        relative_change = (current_time - baseline_time) / baseline_time
        absolute_change = current_time - baseline_time
        
        # Determine status
        if relative_change > threshold:
            status = "REGRESSION"
            has_regression = True
        elif relative_change < -threshold:
            status = "IMPROVEMENT"
        else:
            status = "NEUTRAL"
        
        result = {
            'name': name,
            'baseline_time': baseline_time,
            'current_time': current_time,
            'absolute_change': absolute_change,
            'relative_change': relative_change,
            'relative_change_percent': relative_change * 100,
            'status': status
        }
        
        results.append(result)
    
    return has_regression, results

def print_comparison_report(results: List[Dict[str, Any]], threshold: float):
    """Print a formatted comparison report."""
    print("\n" + "="*80)
    print("BENCHMARK COMPARISON REPORT")
    print("="*80)
    
    # Summary statistics
    regressions = [r for r in results if r['status'] == 'REGRESSION']
    improvements = [r for r in results if r['status'] == 'IMPROVEMENT']
    neutral = [r for r in results if r['status'] == 'NEUTRAL']
    
    print(f"\nSummary:")
    print(f"  Total benchmarks: {len(results)}")
    print(f"  Regressions: {len(regressions)}")
    print(f"  Improvements: {len(improvements)}")
    print(f"  Neutral: {len(neutral)}")
    print(f"  Threshold: ±{threshold*100:.1f}%")
    
    # Detailed results
    print(f"\nDetailed Results:")
    print(f"{'Benchmark':<40} {'Baseline':<12} {'Current':<12} {'Change':<12} {'Status':<12}")
    print("-" * 88)
    
    # Sort by relative change (worst regressions first)
    sorted_results = sorted(results, key=lambda x: x['relative_change'], reverse=True)
    
    for result in sorted_results:
        name = result['name']
        if len(name) > 37:
            name = name[:34] + "..."
        
        baseline_str = f"{result['baseline_time']:.2f}ms"
        current_str = f"{result['current_time']:.2f}ms"
        change_str = f"{result['relative_change_percent']:+.1f}%"
        
        # Color coding for terminal output
        status = result['status']
        if status == 'REGRESSION':
            status_color = f"\033[91m{status}\033[0m"  # Red
        elif status == 'IMPROVEMENT':
            status_color = f"\033[92m{status}\033[0m"  # Green
        else:
            status_color = f"\033[93m{status}\033[0m"  # Yellow
        
        print(f"{name:<40} {baseline_str:<12} {current_str:<12} {change_str:<12} {status_color}")
    
    # Performance statistics
    if results:
        all_changes = [r['relative_change'] for r in results]
        avg_change = statistics.mean(all_changes)
        median_change = statistics.median(all_changes)
        
        print(f"\nPerformance Statistics:")
        print(f"  Average change: {avg_change*100:+.2f}%")
        print(f"  Median change: {median_change*100:+.2f}%")
        
        if regressions:
            worst_regression = max(regressions, key=lambda x: x['relative_change'])
            print(f"  Worst regression: {worst_regression['name']} ({worst_regression['relative_change_percent']:+.1f}%)")
        
        if improvements:
            best_improvement = min(improvements, key=lambda x: x['relative_change'])
            print(f"  Best improvement: {best_improvement['name']} ({best_improvement['relative_change_percent']:+.1f}%)")

def main():
    parser = argparse.ArgumentParser(
        description="Compare benchmark results and detect performance regressions",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python compare_benchmarks.py baseline.json current.json
  python compare_benchmarks.py baseline.json current.json --threshold 0.1
  python compare_benchmarks.py baseline.json current.json --output report.json
        """
    )
    
    parser.add_argument('baseline', help='Baseline benchmark results (JSON file)')
    parser.add_argument('current', help='Current benchmark results (JSON file)')
    parser.add_argument('--threshold', type=float, default=0.05,
                       help='Regression threshold as decimal (default: 0.05 = 5%%)')
    parser.add_argument('--output', help='Save detailed results to JSON file')
    parser.add_argument('--fail-on-regression', action='store_true',
                       help='Exit with non-zero code if regressions detected')
    
    args = parser.parse_args()
    
    # Validate threshold
    if args.threshold <= 0:
        print("Error: Threshold must be positive")
        sys.exit(1)
    
    # Load benchmark data
    print(f"Loading baseline benchmarks from: {args.baseline}")
    baseline_benchmarks = load_benchmark_results(args.baseline)
    baseline_data = extract_benchmark_data(baseline_benchmarks)
    
    print(f"Loading current benchmarks from: {args.current}")
    current_benchmarks = load_benchmark_results(args.current)
    current_data = extract_benchmark_data(current_benchmarks)
    
    print(f"Found {len(baseline_data)} baseline benchmarks")
    print(f"Found {len(current_data)} current benchmarks")
    
    # Compare benchmarks
    has_regression, results = compare_benchmarks(baseline_data, current_data, args.threshold)
    
    # Print report
    print_comparison_report(results, args.threshold)
    
    # Save detailed results if requested
    if args.output:
        output_data = {
            'threshold': args.threshold,
            'summary': {
                'total_benchmarks': len(results),
                'regressions': len([r for r in results if r['status'] == 'REGRESSION']),
                'improvements': len([r for r in results if r['status'] == 'IMPROVEMENT']),
                'neutral': len([r for r in results if r['status'] == 'NEUTRAL']),
                'has_regression': has_regression
            },
            'results': results
        }
        
        with open(args.output, 'w') as f:
            json.dump(output_data, f, indent=2)
        
        print(f"\nDetailed results saved to: {args.output}")
    
    # Final verdict
    print("\n" + "="*80)
    if has_regression:
        print("❌ PERFORMANCE REGRESSION DETECTED!")
        print(f"   One or more benchmarks are >{args.threshold*100:.1f}% slower than baseline")
        
        if args.fail_on_regression:
            sys.exit(1)
    else:
        print("✅ NO SIGNIFICANT PERFORMANCE REGRESSIONS")
        print(f"   All benchmarks are within ±{args.threshold*100:.1f}% of baseline")
    
    print("="*80)

if __name__ == "__main__":
    main()
