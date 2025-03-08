from fastapi import FastAPI
from api import router  # Import API endpoints from api.py

app = FastAPI()

# Include API routes
app.include_router(router)

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="127.0.0.1", port=8000)  # Run only on localhost
