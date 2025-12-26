import { Zap, Shield, Globe, Cpu } from 'lucide-react';

const Features = () => {
    const features = [
        { icon: Zap, title: 'Lightning Fast', desc: 'Process thousands of transactions per second with minimal latency.' },
        { icon: Shield, title: 'Secure by Design', desc: 'Enterprise-grade security with multi-layer protection.' },
        { icon: Globe, title: 'Global Network', desc: 'Decentralized infrastructure across multiple continents.' },
        { icon: Cpu, title: 'Smart Contracts', desc: 'Deploy and execute energy trading contracts seamlessly.' },
    ];

    return (
        <section id="ecosystem" style={{ padding: '80px 24px', background: '#ffffff' }}>
            <div style={{ maxWidth: '1200px', margin: '0 auto' }}>
                <h2 style={{ textAlign: 'center', fontSize: '32px', fontWeight: 700, color: '#0f172a', marginBottom: '16px' }}>
                    Why Choose Hylium?
                </h2>
                <p style={{ textAlign: 'center', color: '#64748b', marginBottom: '48px', maxWidth: '600px', marginLeft: 'auto', marginRight: 'auto' }}>
                    Built for the future of decentralized energy trading
                </p>
                <div style={{ display: 'grid', gridTemplateColumns: 'repeat(4, 1fr)', gap: '24px' }}>
                    {features.map((f, i) => (
                        <div key={i} style={{
                            background: '#f8fafc',
                            borderRadius: '16px',
                            padding: '32px',
                            textAlign: 'center',
                        }}>
                            <div style={{
                                width: '56px',
                                height: '56px',
                                borderRadius: '12px',
                                background: 'linear-gradient(135deg, #00d4ff 0%, #0891b2 100%)',
                                display: 'flex',
                                alignItems: 'center',
                                justifyContent: 'center',
                                margin: '0 auto 16px',
                            }}>
                                <f.icon size={28} color="white" />
                            </div>
                            <h3 style={{ fontSize: '18px', fontWeight: 600, color: '#0f172a', marginBottom: '8px' }}>{f.title}</h3>
                            <p style={{ color: '#64748b', fontSize: '14px', lineHeight: 1.6 }}>{f.desc}</p>
                        </div>
                    ))}
                </div>
            </div>
            <style>{`@media (max-width: 900px) { section > div > div:last-child { grid-template-columns: repeat(2, 1fr) !important; } }`}</style>
        </section>
    );
};

export default Features;
