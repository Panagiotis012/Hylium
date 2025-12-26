import { Github, Twitter, MessageCircle } from 'lucide-react';

const Footer = () => {
    return (
        <footer style={{
            backgroundColor: '#ffffff',
            borderTop: '1px solid #e2e8f0',
            padding: '48px 24px 24px',
        }}>
            <div style={{ maxWidth: '1200px', margin: '0 auto' }}>
                <div style={{ display: 'grid', gridTemplateColumns: 'repeat(4, 1fr)', gap: '32px', marginBottom: '32px' }}>
                    {/* Brand */}
                    <div>
                        <div style={{ display: 'flex', alignItems: 'center', gap: '10px', marginBottom: '16px' }}>
                            <img src="/logo.png" alt="Hylium" style={{ width: '32px', height: '32px' }} />
                            <span style={{ fontWeight: 700, fontSize: '18px', color: '#0f172a' }}>Hylium</span>
                        </div>
                        <p style={{ color: '#64748b', fontSize: '14px', lineHeight: 1.6 }}>
                            Decentralized energy infrastructure for the future.
                        </p>
                    </div>

                    {/* Links */}
                    {[
                        { title: 'Protocol', links: ['Ecosystem', 'Technology', 'Roadmap'] },
                        { title: 'Resources', links: ['Documentation', 'Whitepaper', 'GitHub'] },
                        { title: 'Community', links: ['Discord', 'Twitter', 'Telegram'] },
                    ].map((col, i) => (
                        <div key={i}>
                            <h4 style={{ fontWeight: 600, color: '#0f172a', marginBottom: '16px' }}>{col.title}</h4>
                            {col.links.map((link, j) => (
                                <a key={j} href="#" style={{ display: 'block', color: '#64748b', fontSize: '14px', marginBottom: '8px' }}>
                                    {link}
                                </a>
                            ))}
                        </div>
                    ))}
                </div>

                {/* Bottom */}
                <div style={{
                    borderTop: '1px solid #e2e8f0',
                    paddingTop: '24px',
                    display: 'flex',
                    justifyContent: 'space-between',
                    alignItems: 'center',
                }}>
                    <p style={{ color: '#94a3b8', fontSize: '14px' }}>© 2024 Hylium. All rights reserved.</p>
                    <div style={{ display: 'flex', gap: '16px' }}>
                        <a href="#" style={{ color: '#94a3b8' }}><Github size={20} /></a>
                        <a href="#" style={{ color: '#94a3b8' }}><Twitter size={20} /></a>
                        <a href="#" style={{ color: '#94a3b8' }}><MessageCircle size={20} /></a>
                    </div>
                </div>
            </div>
            <style>{`@media (max-width: 768px) { footer > div > div:first-child { grid-template-columns: repeat(2, 1fr) !important; } }`}</style>
        </footer>
    );
};

export default Footer;
