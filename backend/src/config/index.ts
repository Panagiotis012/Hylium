import dotenv from 'dotenv';
dotenv.config();

export const config = {
    env: process.env.NODE_ENV || 'development',
    port: parseInt(process.env.PORT || '3001', 10),

    jwt: {
        secret: process.env.JWT_SECRET || 'change-me',
        refreshSecret: process.env.JWT_REFRESH_SECRET || 'change-me-refresh',
        expiresIn: process.env.JWT_EXPIRES_IN || '15m',
        refreshExpiresIn: process.env.JWT_REFRESH_EXPIRES_IN || '7d',
    },

    hyliumRpc: {
        url: process.env.HYLIUM_RPC_URL || 'http://127.0.0.1:8332',
        user: process.env.HYLIUM_RPC_USER || '',
        pass: process.env.HYLIUM_RPC_PASS || '',
    },

    cors: {
        origin: process.env.CORS_ORIGIN || 'http://localhost:5173',
    },

    rateLimit: {
        windowMs: parseInt(process.env.RATE_LIMIT_WINDOW_MS || '900000', 10),
        max: parseInt(process.env.RATE_LIMIT_MAX || '100', 10),
    },
};
