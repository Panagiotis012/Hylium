import { ArrowRight, FileText } from 'lucide-react';

const Hero = () => {
    return (
        <section style={{
            minHeight: '100vh',
            display: 'flex',
            alignItems: 'center',
            paddingTop: '80px',
        }}>
            <div style={{
                maxWidth: '1280px',
                margin: '0 auto',
                padding: '64px 24px',
                display: 'grid',
                gridTemplateColumns: 'repeat(2, 1fr)',
                gap: '48px',
                alignItems: 'center',
            }}>
                {/* Left Column - Text */}
                <div style={{ display: 'flex', flexDirection: 'column', gap: '24px' }}>
                    {/* Tagline */}
                    <p style={{
                        color: '#00f2ea',
                        fontWeight: 600,
                        letterSpacing: '0.1em',
                        fontSize: '14px',
                        textTransform: 'uppercase',
                    }}>
                        THE NEXT GENERATION PROTOCOL
                    </p>

                    {/* Headline */}
                    <h1 style={{
                        fontSize: 'clamp(32px, 5vw, 56px)',
                        fontWeight: 800,
                        color: 'white',
                        lineHeight: 1.1,
                    }}>
                        THE FOUNDATION LAYER FOR DECENTRALIZED ENERGY.
                    </h1>

                    {/* Subtitle */}
                    <p style={{
                        color: '#9ca3af',
                        fontSize: '18px',
                        maxWidth: '500px',
                        lineHeight: 1.7,
                    }}>
                        Hylium provides the infrastructure for a new era of decentralized energy markets,
                        enabling transparent, secure, and efficient energy trading across the globe.
                    </p>

                    {/* Buttons */}
                    <div style={{
                        display: 'flex',
                        flexWrap: 'wrap',
                        gap: '16px',
                        paddingTop: '16px'
                    }}>
                        {/* Primary Button with Glow */}
                        <button style={{
                            padding: '16px 32px',
                            backgroundColor: '#00f2ea',
                            color: 'black',
                            fontWeight: 600,
                            borderRadius: '9999px',
                            border: 'none',
                            display: 'flex',
                            alignItems: 'center',
                            gap: '8px',
                            cursor: 'pointer',
                            boxShadow: '0 0 30px rgba(0, 242, 234, 0.5), 0 0 60px rgba(0, 242, 234, 0.3)',
                            transition: 'transform 0.2s',
                        }}
                            onMouseOver={(e) => (e.currentTarget.style.transform = 'scale(1.05)')}
                            onMouseOut={(e) => (e.currentTarget.style.transform = 'scale(1)')}
                        >
                            JOIN PRESALE
                            <ArrowRight size={18} />
                        </button>

                        {/* Secondary Button */}
                        <button style={{
                            padding: '16px 32px',
                            border: '1px solid rgba(255, 255, 255, 0.3)',
                            backgroundColor: 'transparent',
                            color: 'white',
                            fontWeight: 500,
                            borderRadius: '9999px',
                            display: 'flex',
                            alignItems: 'center',
                            gap: '8px',
                            cursor: 'pointer',
                            transition: 'all 0.2s',
                        }}
                            onMouseOver={(e) => (e.currentTarget.style.backgroundColor = 'rgba(255, 255, 255, 0.05)')}
                            onMouseOut={(e) => (e.currentTarget.style.backgroundColor = 'transparent')}
                        >
                            <FileText size={18} />
                            Read Technical Paper
                        </button>
                    </div>
                </div>

                {/* Right Column - Visual Placeholder */}
                <div style={{
                    display: 'flex',
                    alignItems: 'center',
                    justifyContent: 'center',
                    position: 'relative',
                }}>
                    <div style={{
                        width: '100%',
                        maxWidth: '400px',
                        aspectRatio: '1',
                        position: 'relative',
                    }}>
                        {/* Glow Background */}
                        <div style={{
                            position: 'absolute',
                            inset: 0,
                            background: 'radial-gradient(circle, rgba(0, 242, 234, 0.2) 0%, transparent 70%)',
                            borderRadius: '50%',
                            filter: 'blur(40px)',
                        }}></div>

                        {/* Animated Rings */}
                        <div style={{
                            position: 'absolute',
                            inset: '10%',
                            border: '2px solid rgba(0, 242, 234, 0.3)',
                            borderRadius: '50%',
                            animation: 'pulse 2s ease-in-out infinite',
                        }}></div>
                        <div style={{
                            position: 'absolute',
                            inset: '25%',
                            border: '1px solid rgba(0, 242, 234, 0.2)',
                            borderRadius: '50%',
                        }}></div>
                        <div style={{
                            position: 'absolute',
                            inset: '40%',
                            backgroundColor: 'rgba(0, 242, 234, 0.1)',
                            borderRadius: '50%',
                        }}></div>

                        {/* Center Logo */}
                        <div style={{
                            position: 'absolute',
                            top: '50%',
                            left: '50%',
                            transform: 'translate(-50%, -50%)',
                            width: '80px',
                            height: '80px',
                            backgroundColor: '#00f2ea',
                            borderRadius: '16px',
                            display: 'flex',
                            alignItems: 'center',
                            justifyContent: 'center',
                            boxShadow: '0 0 40px rgba(0, 242, 234, 0.5)',
                        }}>
                            <span style={{ color: 'black', fontWeight: 'bold', fontSize: '32px' }}>H</span>
                        </div>
                    </div>
                </div>
            </div>

            <style>{`
        @keyframes pulse {
          0%, 100% { opacity: 0.3; transform: scale(1); }
          50% { opacity: 0.6; transform: scale(1.05); }
        }
        
        @media (max-width: 768px) {
          section > div {
            grid-template-columns: 1fr !important;
            text-align: center;
          }
        }
      `}</style>
        </section>
    );
};

export default Hero;
