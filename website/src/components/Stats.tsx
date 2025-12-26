import { useEffect, useState } from 'react';
import { getStats } from '../services/api';
import type { NetworkStats } from '../services/api';

const Stats = () => {
    const [stats, setStats] = useState<NetworkStats | null>(null);

    useEffect(() => {
        const fetchStats = async () => {
            const { data } = await getStats();
            if (data) setStats(data);
        };
        fetchStats();
        const interval = setInterval(fetchStats, 30000);
        return () => clearInterval(interval);
    }, []);

    const items = [
        { label: 'Block Height', value: stats?.blockHeight?.toLocaleString() || '0' },
        { label: 'Network Hashrate', value: `${Math.round((stats?.networkHashrate || 0) / 1000000)} MH/s` },
        { label: 'Difficulty', value: stats?.difficulty?.toFixed(2) || '0' },
        { label: 'Connections', value: stats?.connections?.toString() || '0' },
    ];

    return (
        <section id="stats" style={{ padding: '80px 24px', background: '#f8fafc' }}>
            <div style={{ maxWidth: '1200px', margin: '0 auto' }}>
                <h2 style={{ textAlign: 'center', fontSize: '32px', fontWeight: 700, color: '#0f172a', marginBottom: '48px' }}>
                    Network Statistics
                </h2>
                <div style={{ display: 'grid', gridTemplateColumns: 'repeat(4, 1fr)', gap: '24px' }}>
                    {items.map((item, i) => (
                        <div key={i} style={{
                            background: '#ffffff',
                            borderRadius: '16px',
                            padding: '32px',
                            textAlign: 'center',
                            border: '1px solid #e2e8f0',
                        }}>
                            <p style={{ fontSize: '36px', fontWeight: 700, color: '#00d4ff' }}>{item.value}</p>
                            <p style={{ color: '#64748b', fontSize: '14px', marginTop: '8px' }}>{item.label}</p>
                        </div>
                    ))}
                </div>
            </div>
            <style>{`@media (max-width: 768px) { section > div > div { grid-template-columns: repeat(2, 1fr) !important; } }`}</style>
        </section>
    );
};

export default Stats;
