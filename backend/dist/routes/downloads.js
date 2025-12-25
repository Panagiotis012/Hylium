import { Router } from 'express';
import { PrismaClient } from '@prisma/client';
import { validate, schemas } from '../middleware/validate.js';
import { authenticate, requireAdmin } from '../middleware/auth.js';
const router = Router();
const prisma = new PrismaClient();
// Get all downloads (public)
router.get('/', async (_req, res) => {
    try {
        const downloads = await prisma.download.findMany({
            orderBy: { createdAt: 'desc' },
        });
        res.json(downloads);
    }
    catch (error) {
        console.error('Downloads error:', error);
        res.status(500).json({ error: 'Internal server error' });
    }
});
// Add download (admin only)
router.post('/', authenticate, requireAdmin, validate(schemas.download), async (req, res) => {
    try {
        const { name, version, platform, url, checksum } = req.body;
        const download = await prisma.download.create({
            data: { name, version, platform, url, checksum },
        });
        res.status(201).json(download);
    }
    catch (error) {
        console.error('Add download error:', error);
        res.status(500).json({ error: 'Internal server error' });
    }
});
// Delete download (admin only)
router.delete('/:id', authenticate, requireAdmin, async (req, res) => {
    try {
        const { id } = req.params;
        await prisma.download.delete({ where: { id } });
        res.json({ message: 'Download deleted' });
    }
    catch (error) {
        console.error('Delete download error:', error);
        res.status(500).json({ error: 'Internal server error' });
    }
});
export default router;
//# sourceMappingURL=downloads.js.map