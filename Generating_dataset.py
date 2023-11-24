import pandas as pd
import numpy as np
import random
from sklearn.preprocessing import MinMaxScaler
from scipy.stats import zscore

# Constants
product_names = ['OnePlus', 'Samsung', 'iPhone']
product_prices = {'OnePlus': 60000, 'Samsung': 75000, 'iPhone': 90000}
colors = ['Red', 'Blue', 'Green']
num_samples = 1300

# Generate random data
data = {
    'timestamp': pd.date_range(start='2020-01-01', periods=num_samples, freq='D'),
    'quantity': [random.randint(1, 200) for _ in range(num_samples)],
    'price': [product_prices[random.choice(product_names)] for _ in range(num_samples)],
    'feature1': [random.uniform(-3, 3) for _ in range(num_samples)],
    'feature2': [random.choice(colors) for _ in range(num_samples)]
}

# Create DataFrame
df = pd.DataFrame(data)

# Apply transformations
scaler = MinMaxScaler()
df['feature1_scaled'] = scaler.fit_transform(df[['feature1']])

bins = [-float('inf'), -1, 1, float('inf')]
labels = ['low', 'medium', 'high']
df['feature1_binned'] = pd.cut(df['feature1'], bins=bins, labels=labels, include_lowest=True)

df['feature1_squared'] = df['feature1'] ** 2
df['feature1_log'] = np.log1p(df['feature1'])

df['interaction_feature1_price'] = df['feature1'] * df['price']

# Remove outliers using z-score
z_scores = zscore(df['feature1'])
df['feature1_no_outliers'] = df[(z_scores < 3) & (z_scores > -3)]['feature1']

# Save to CSV file
df.to_csv('generated_data_transformed.csv', index=False)

print("Transformed data saved to 'generated_data_transformed.csv'")
