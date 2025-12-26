import { Shield, Newspaper, Activity, Server } from 'lucide-react';

const TrustFooter = () => {
    const items = [
        { icon: Shield, label: 'AUDITED BY', value: 'Certik' },
        { icon: Newspaper, label: 'FEATURED IN', value: 'CoinDesk' },
        { icon: Activity, label: 'TOTAL TRANSACTIONS', value: '1.2M+' },
        { icon: Server, label: 'ACTIVE NODES', value: '4,500' },
    ];

    return (
        <section style={{
            backgroundColor: 'rgba(255, 255, 255, 0.7)',
            backdropFilter: 'blur(10px)',
            borderTop: '1px solid rgba(0, 0, 0, 0.05)',
            position: 'fixed',
            bottom: 0,
            left: 0,
            right: 0,
        }}>
            <div style={{
                maxWidth: '1280px',
                margin: '0 auto',
                padding: '24px',
                display: 'grid',
                gridTemplateColumns: 'repeat(4, 1fr)',
                gap: '32px',
            }}>
                {items.map((item, index) => (
                    <div key={index} style={{
                        display: 'flex',
                        alignItems: 'center',
                        gap: '12px'
                    }}>
                        <item.icon style={{ width: '24px', height: '24px', color: '#94a3b8' }} />
                        <div>
                            <p style={{
                                color: '#94a3b8',
                                fontSize: '11px',
                                textTransform: 'uppercase',
                                letterSpacing: '0.05em'
                            }}>
                                {item.label}
                            </p>
                            <p style={{ color: '#0f172a', fontWeight: 600 }}>{item.value}</p>
                        </div>
                    </div>
                ))}
            </div>

            <style>{`
        @media (max-width: 768px) {
          section > div {
            grid-template-columns: repeat(2, 1fr) !important;
          }
        }
      `}</style>
        </section>
    );
};

export default TrustFooter;
