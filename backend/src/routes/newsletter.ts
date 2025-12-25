import { Router, Request, Response } from 'express';
import { PrismaClient } from '@prisma/client';
import { validate, schemas } from '../middleware/validate.js';

const router = Router();
const prisma = new PrismaClient();

// Subscribe to newsletter
router.post('/', validate(schemas.newsletter), async (req: Request, res: Response) => {
    try {
        const { email } = req.body;

        const existing = await prisma.newsletter.findUnique({ where: { email } });
        if (existing) {
            res.status(200).json({ message: 'Already subscribed' });
            return;
        }

        await prisma.newsletter.create({ data: { email } });

        res.status(201).json({ message: 'Subscribed successfully' });
    } catch (error) {
        console.error('Newsletter error:', error);
        res.status(500).json({ error: 'Internal server error' });
    }
});

export default router;
