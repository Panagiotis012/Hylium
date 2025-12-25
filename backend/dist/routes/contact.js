import { Router } from 'express';
import { PrismaClient } from '@prisma/client';
import { validate, schemas } from '../middleware/validate.js';
const router = Router();
const prisma = new PrismaClient();
// Submit contact form
router.post('/', validate(schemas.contact), async (req, res) => {
    try {
        const { email, subject, message } = req.body;
        await prisma.contact.create({
            data: { email, subject, message },
        });
        res.status(201).json({ message: 'Message sent successfully' });
    }
    catch (error) {
        console.error('Contact error:', error);
        res.status(500).json({ error: 'Internal server error' });
    }
});
export default router;
//# sourceMappingURL=contact.js.map