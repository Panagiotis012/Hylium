import { Router } from 'express';
import { config } from '../config/index.js';
import { validate, schemas } from '../middleware/validate.js';
const router = Router();
// Whitelisted RPC methods (safe, read-only)
const ALLOWED_METHODS = new Set([
    'getblockchaininfo',
    'getblockcount',
    'getdifficulty',
    'getnetworkhashps',
    'getmininginfo',
    'getpeerinfo',
    'getnettotals',
    'getconnectioncount',
    'getbestblockhash',
    'getblock',
    'getblockhash',
    'getrawtransaction',
    'decoderawtransaction',
]);
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
    return data;
}
// RPC Proxy (whitelisted methods only)
router.post('/', validate(schemas.rpc), async (req, res) => {
    try {
        const { method, params } = req.body;
        // Security: Only allow whitelisted methods
        if (!ALLOWED_METHODS.has(method.toLowerCase())) {
            res.status(403).json({
                error: 'Method not allowed',
                allowedMethods: Array.from(ALLOWED_METHODS),
            });
            return;
        }
        const result = await callRpc(method, params);
        res.json(result);
    }
    catch (error) {
        console.error('RPC error:', error);
        res.status(500).json({ error: 'RPC call failed' });
    }
});
export default router;
//# sourceMappingURL=rpc.js.map