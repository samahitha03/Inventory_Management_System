import streamlit as st
import pandas as pd
import numpy as np
from sklearn.model_selection import train_test_split
from sklearn.ensemble import RandomForestRegressor
from sklearn.preprocessing import StandardScaler, OneHotEncoder
from sklearn.compose import ColumnTransformer
from sklearn.pipeline import Pipeline
from sklearn.impute import SimpleImputer
from sklearn.metrics import mean_absolute_error
from scipy.stats import zscore 
import matplotlib.pyplot as plt
import seaborn as sns

file_path = r"File_To_CSV_File"
df = pd.read_csv(file_path)


df['timestamp'] = pd.to_datetime(df['timestamp'])
df['day_of_week'] = df['timestamp'].dt.dayofweek
df['month'] = df['timestamp'].dt.month

# Separate features and target variable
X = df[['price', 'feature1_scaled', 'feature1_binned', 'feature1_squared', 'feature1_log', 'interaction_feature1_price', 'feature1_no_outliers', 'day_of_week', 'month', 'feature2']]
y = df['quantity']

# Split the data into training and testing sets
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)

preprocessor = ColumnTransformer(
    transformers=[
        ('num', StandardScaler(), ['price', 'feature1_scaled', 'feature1_squared', 'feature1_log', 'interaction_feature1_price', 'feature1_no_outliers', 'day_of_week', 'month']),
        ('cat', OneHotEncoder(), ['feature1_binned', 'feature2'])
    ])

model = Pipeline(steps=[('preprocessor', preprocessor),
                        ('imputer', SimpleImputer(strategy='mean')),
                        ('regressor', RandomForestRegressor(n_estimators=100, random_state=42))])

# Train the model
model.fit(X_train, y_train)


def preprocess_input(product, timestamp, feature1, feature2):
    timestamp = pd.to_datetime(timestamp)
    day_of_week = timestamp.dayofweek
    month = timestamp.month

    # Map product to its associated price
    product_prices = {'iphone': 60000, 'samsung': 75000, 'oneplus': 90000}
    price = product_prices.get(product.lower(), 0)  # Default to 0 if product not found

    # Apply the same transformations to feature1 as in the training data
    feature1_scaled = (feature1 - df['feature1'].min()) / (df['feature1'].max() - df['feature1'].min())
    feature1_binned = pd.cut([feature1], bins=[-float('inf'), -1, 1, float('inf')], labels=['low', 'medium', 'high'], include_lowest=True)[0]
    feature1_squared = feature1 ** 2
    feature1_log = np.log1p(feature1)
    interaction_feature1_price = feature1 * price
    z_score = zscore([feature1])[0]
    feature1_no_outliers = feature1 if -3 <= z_score <= 3 else np.nan

    return pd.DataFrame({
        'price': [price],
        'feature1_scaled': [feature1_scaled],
        'feature1_binned': [feature1_binned],
        'feature1_squared': [feature1_squared],
        'feature1_log': [feature1_log],
        'interaction_feature1_price': [interaction_feature1_price],
        'feature1_no_outliers': [feature1_no_outliers],
        'day_of_week': [day_of_week],
        'month': [month],
        'feature2': [feature2]
    })

# Streamlit UI
st.title('Product Quantity Prediction')
st.write("""The feature below allows users to input values and predict the amount of products present in the store or factory at a given date.""")

# User inputs
product = st.selectbox('Select Product:', ['iPhone', 'Samsung', 'OnePlus'])
timestamp = st.date_input('Enter Timestamp:')
st.write("""The feature 1 indicates trend rate""")
feature1 = st.slider('Select Feature 1:', min_value=df['feature1'].min(), max_value=df['feature1'].max(), step=0.1)
feature2 = st.selectbox('Select Feature 2:', df['feature2'].unique())

# Make prediction when the user clicks the button
if st.button('Predict Quantity'):
    input_data = preprocess_input(product, timestamp, feature1, feature2)
    prediction = int(round(model.predict(input_data)[0]))
    st.success(f'Predicted Quantity: {prediction}')

