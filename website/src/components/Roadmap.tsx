import { CheckCircle, Circle } from 'lucide-react';

const Roadmap = () => {
    const phases = [
        { q: 'Q1 2024', title: 'Foundation', items: ['Core protocol development', 'Testnet launch', 'Security audits'], done: true },
        { q: 'Q2 2024', title: 'Growth', items: ['Mainnet launch', 'Wallet integration', 'First partnerships'], done: true },
        { q: 'Q3 2024', title: 'Expansion', items: ['DEX integration', 'Mobile app beta', 'Staking rewards'], done: false },
        { q: 'Q4 2024', title: 'Scale', items: ['Enterprise solutions', 'Cross-chain bridges', 'Global expansion'], done: false },
    ];

    return (
        <section id="roadmap" style={{ padding: '80px 24px', background: '#f8fafc' }}>
            <div style={{ maxWidth: '1200px', margin: '0 auto' }}>
                <h2 style={{ textAlign: 'center', fontSize: '32px', fontWeight: 700, color: '#0f172a', marginBottom: '48px' }}>
                    Roadmap
                </h2>
                <div style={{ display: 'grid', gridTemplateColumns: 'repeat(4, 1fr)', gap: '24px' }}>
                    {phases.map((p, i) => (
                        <div key={i} style={{
                            background: '#ffffff',
                            borderRadius: '16px',
                            padding: '28px',
                            border: p.done ? '2px solid #00d4ff' : '1px solid #e2e8f0',
                        }}>
                            <p style={{ color: '#00d4ff', fontWeight: 600, fontSize: '13px', marginBottom: '8px' }}>{p.q}</p>
                            <h3 style={{ fontSize: '20px', fontWeight: 700, color: '#0f172a', marginBottom: '16px' }}>{p.title}</h3>
                            {p.items.map((item, j) => (
                                <div key={j} style={{ display: 'flex', alignItems: 'center', gap: '8px', marginBottom: '8px' }}>
                                    {p.done ? <CheckCircle size={16} color="#00d4ff" /> : <Circle size={16} color="#cbd5e1" />}
                                    <span style={{ color: '#64748b', fontSize: '14px' }}>{item}</span>
                                </div>
                            ))}
                        </div>
                    ))}
                </div>
            </div>
            <style>{`@media (max-width: 900px) { section > div > div:last-child { grid-template-columns: repeat(2, 1fr) !important; } }`}</style>
        </section>
    );
};

export default Roadmap;
