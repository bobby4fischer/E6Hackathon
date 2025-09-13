import csv
import random

print("Generating large dataset...")
headers = ['category', 'value']
categories = ['A', 'B', 'C', 'D', 'E']

with open('data/large_data.csv', 'w', newline='') as f:
    writer = csv.writer(f)
    writer.writerow(headers)
    for _ in range(5_000_000):  # Generate 5 million rows
        row = [
            random.choice(categories),
            random.randint(50, 500)
        ]
        writer.writerow(row)

print("Done.")z