import { Router } from 'express';
import { config } from '../config/index.js';
const router = Router();
// Cache for stats
let statsCache = null;
const CACHE_TTL = 30000; // 30 seconds
async function callRpc(method, params = []) {
    const { url, user, pass } = config.hyliumRpc;
    const response = await fetch(url, {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
            'Authorization': 'Basic ' + Buffer.from(`${user}:${pass}`).toString('base64'),
        },
        body: JSON.stringify({
            jsonrpc: '1.0',
            id: 'hylium-web',
            method,
            params,
        }),
    });
    const data = await response.json();
    if (data.error) {
        throw new Error(data.error.message || 'RPC error');
    }
    return data.result;
}
// Public stats endpoint (cached)
router.get('/', async (_req, res) => {
    try {
        // Check cache
        if (statsCache && Date.now() - statsCache.timestamp < CACHE_TTL) {
            res.json(statsCache.data);
            return;
        }
        // Fetch fresh data
        const [blockchainInfo, miningInfo, connectionCount] = await Promise.all([
            callRpc('getblockchaininfo').catch(() => null),
            callRpc('getmininginfo').catch(() => null),
            callRpc('getconnectioncount').catch(() => null),
        ]);
        const stats = {
            blockHeight: blockchainInfo?.blocks || 0,
            difficulty: blockchainInfo?.difficulty || 0,
            chain: blockchainInfo?.chain || 'unknown',
            networkHashrate: miningInfo?.networkhashps || 0,
            connections: connectionCount || 0,
            lastUpdated: new Date().toISOString(),
        };
        // Update cache
        statsCache = { data: stats, timestamp: Date.now() };
        res.json(stats);
    }
    catch (error) {
        console.error('Stats error:', error);
        // Return cached data if available, even if stale
        if (statsCache) {
            res.json({ ...statsCache.data, stale: true });
            return;
        }
        res.status(503).json({ error: 'Unable to fetch network stats' });
    }
});
export default router;
//# sourceMappingURL=stats.js.map