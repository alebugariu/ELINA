#!/usr/bin/python3

import csv
import sys

def dump_result(writer, results):
	if results is not None:
		results['status'] = (
			"PASS"
			if results['error message'] == '-'
			else "FAIL")
		print(results)
		writer.writerow([results[column] for column in columns])

if len(sys.argv) != 3:
    print("{} input output".format(sys.argv[0]))
    sys.exit(1)
else:
    _, input_file, output_file = sys.argv

with open(output_file, 'w') as results_fp:
	writer = csv.writer(results_fp)
	columns = [
	    'test case',
	    'execution time (seconds)',
	    'execution time (minutes)',
		'completed paths',
		'generated tests',
		'total instructions',
		'status',
		'error message',
		'output directory'
		]
	writer.writerow(columns)
	for test_file in [input_file]:
		results = None
		with open(test_file) as fp:
			for line in fp:
				if line.startswith('KLEE: output directory is '):
				    value = line.replace('KLEE: output directory is ', '').strip()
				    dump_result(writer, results)
				    results = {}
				    results['error message']= '-'
				    results['completed paths']= '-'
				    results['generated tests']= '-'
				    results['total instructions'] = '-'
				    results['output directory'] = value[1:-1]
				elif line.startswith('KLEE: done: '):
					key, value = line.split(' = ')
					key = key.replace('KLEE: done: ', '')
					results[key] = int(value.strip())
				elif line.startswith('Execution time: '):
				   line = line.replace('Execution time: ', '').strip();
				   values = line.split(" ");
				   results['execution time (seconds)'] = int(values[0])
				   results['execution time (minutes)'] = float(values[0]) / 60
				   results['test case'] = values[-1]
				elif 'KLEE: ERROR: ' in line:
					ind = line.index('KLEE: ERROR: ')
					results['error message'] = line[ind:].strip()
				elif 'Segmentation fault' in line:
                                        results['error message'] = 'Segmentation fault'
		dump_result(writer, results)
