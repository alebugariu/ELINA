#!/usr/bin/python3

import csv

with open('results.csv', 'w') as results_fp:
	writer = csv.writer(results_fp)
	columns = [
		'completed paths',
		'generated tests',
		'total instructions',
		]
	writer.writerow(columns)
	for test_file in ['results.txt']:
		results = {}
		with open(test_file) as fp:
			for line in fp:
				if line.startswith('KLEE: done: '):
					key, value = line.split(' = ')
					key = key.replace('KLEE: done: ', '')
					results[key] = int(value.strip())
		writer.writerow([results[column] for column in columns])
