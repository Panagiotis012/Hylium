import { Router } from 'express';
import bcrypt from 'bcryptjs';
import jwt from 'jsonwebtoken';
import { PrismaClient } from '@prisma/client';
import { config } from '../config/index.js';
import { validate, schemas } from '../middleware/validate.js';
import { authenticate } from '../middleware/auth.js';
const router = Router();
const prisma = new PrismaClient();
// Register
router.post('/register', validate(schemas.register), async (req, res) => {
    try {
        const { email, password } = req.body;
        const existingUser = await prisma.user.findUnique({ where: { email } });
        if (existingUser) {
            res.status(400).json({ error: 'Email already registered' });
            return;
        }
        const hashedPassword = await bcrypt.hash(password, 12);
        const user = await prisma.user.create({
            data: { email, password: hashedPassword },
        });
        res.status(201).json({
            message: 'User created successfully',
            userId: user.id,
        });
    }
    catch (error) {
        console.error('Register error:', error);
        res.status(500).json({ error: 'Internal server error' });
    }
});
// Login
router.post('/login', validate(schemas.login), async (req, res) => {
    try {
        const { email, password } = req.body;
        const user = await prisma.user.findUnique({ where: { email } });
        if (!user) {
            res.status(401).json({ error: 'Invalid credentials' });
            return;
        }
        const validPassword = await bcrypt.compare(password, user.password);
        if (!validPassword) {
            res.status(401).json({ error: 'Invalid credentials' });
            return;
        }
        const accessToken = jwt.sign({ userId: user.id, role: user.role }, config.jwt.secret, { expiresIn: '15m' });
        const refreshToken = jwt.sign({ userId: user.id }, config.jwt.refreshSecret, { expiresIn: '7d' });
        res.json({ accessToken, refreshToken });
    }
    catch (error) {
        console.error('Login error:', error);
        res.status(500).json({ error: 'Internal server error' });
    }
});
// Refresh token
router.post('/refresh', async (req, res) => {
    try {
        const { refreshToken } = req.body;
        if (!refreshToken) {
            res.status(401).json({ error: 'Refresh token required' });
            return;
        }
        const decoded = jwt.verify(refreshToken, config.jwt.refreshSecret);
        const user = await prisma.user.findUnique({ where: { id: decoded.userId } });
        if (!user) {
            res.status(401).json({ error: 'Invalid refresh token' });
            return;
        }
        const accessToken = jwt.sign({ userId: user.id, role: user.role }, config.jwt.secret, { expiresIn: '15m' });
        res.json({ accessToken });
    }
    catch (error) {
        res.status(401).json({ error: 'Invalid refresh token' });
    }
});
// Get current user
router.get('/me', authenticate, async (req, res) => {
    try {
        const user = await prisma.user.findUnique({
            where: { id: req.userId },
            select: { id: true, email: true, role: true, createdAt: true },
        });
        if (!user) {
            res.status(404).json({ error: 'User not found' });
            return;
        }
        res.json(user);
    }
    catch (error) {
        res.status(500).json({ error: 'Internal server error' });
    }
});
export default router;
//# sourceMappingURL=auth.js.map