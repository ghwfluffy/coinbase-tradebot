# Get a JWT

from coinbase import jwt_generator

api_key = "API Key"
api_secret = "ECC Key"

request_method = "GET"
request_path = "/api/v3/brokerage/accounts"

jwt_uri = jwt_generator.format_jwt_uri(request_method, request_path)
jwt_token = jwt_generator.build_rest_jwt(jwt_uri, api_key, api_secret)
print(f"export JWT={jwt_token}")
