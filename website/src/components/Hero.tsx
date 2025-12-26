import { ArrowRight, FileText } from 'lucide-react';
import { Suspense } from 'react';
import Scene3D from './Scene3D';

const Hero = () => {
    return (
        <section id="home" style={{
            minHeight: '100vh',
            display: 'flex',
            alignItems: 'center',
            paddingTop: '80px',
            background: 'linear-gradient(180deg, #ffffff 0%, #f0f9ff 100%)',
        }}>
            <div style={{
                maxWidth: '1400px',
                margin: '0 auto',
                padding: '40px 24px',
                display: 'grid',
                gridTemplateColumns: '1fr 1.5fr',
                gap: '24px',
                alignItems: 'center',
            }}>
                {/* Left Column - Text */}
                <div style={{ display: 'flex', flexDirection: 'column', gap: '20px' }}>
                    <p style={{
                        color: '#0891b2',
                        fontWeight: 600,
                        letterSpacing: '0.15em',
                        fontSize: '13px',
                        textTransform: 'uppercase',
                    }}>
                        NEXT GENERATION PROTOCOL
                    </p>

                    <h1 style={{
                        fontSize: 'clamp(28px, 4vw, 48px)',
                        fontWeight: 800,
                        color: '#0f172a',
                        lineHeight: 1.15,
                    }}>
                        The Foundation Layer for Decentralized Energy
                    </h1>

                    <p style={{
                        color: '#64748b',
                        fontSize: '17px',
                        lineHeight: 1.7,
                    }}>
                        Hylium powers the next era of decentralized energy markets with secure, transparent, and efficient infrastructure.
                    </p>

                    <div style={{ display: 'flex', flexWrap: 'wrap', gap: '12px', paddingTop: '12px' }}>
                        <button style={{
                            padding: '14px 28px',
                            backgroundColor: '#00d4ff',
                            color: '#0f172a',
                            fontWeight: 600,
                            borderRadius: '12px',
                            border: 'none',
                            display: 'flex',
                            alignItems: 'center',
                            gap: '8px',
                            cursor: 'pointer',
                            fontSize: '15px',
                        }}>
                            Join Presale <ArrowRight size={18} />
                        </button>

                        <button style={{
                            padding: '14px 28px',
                            border: '2px solid #e2e8f0',
                            backgroundColor: '#ffffff',
                            color: '#0f172a',
                            fontWeight: 500,
                            borderRadius: '12px',
                            display: 'flex',
                            alignItems: 'center',
                            gap: '8px',
                            cursor: 'pointer',
                            fontSize: '15px',
                        }}>
                            <FileText size={18} /> Whitepaper
                        </button>
                    </div>
                </div>

                {/* Right Column - 3D Model (VERY BIG) */}
                <div style={{ width: '100%', height: '650px', position: 'relative' }}>
                    <Suspense fallback={
                        <div style={{ width: '100%', height: '100%', display: 'flex', alignItems: 'center', justifyContent: 'center', color: '#0891b2' }}>
                            Loading...
                        </div>
                    }>
                        <Scene3D />
                    </Suspense>
                </div>
            </div>

            <style>{`
        @media (max-width: 900px) {
          section > div { grid-template-columns: 1fr !important; text-align: center; }
        }
      `}</style>
        </section>
    );
};

export default Hero;
