import { Menu, X } from 'lucide-react';
import { useState } from 'react';

const Navbar = () => {
    const [isOpen, setIsOpen] = useState(false);

    return (
        <nav style={{
            position: 'fixed',
            top: 0,
            left: 0,
            right: 0,
            zIndex: 50,
            backgroundColor: 'rgba(0, 0, 0, 0.5)',
            backdropFilter: 'blur(20px)',
            borderBottom: '1px solid rgba(255, 255, 255, 0.05)',
        }}>
            <div style={{
                maxWidth: '1280px',
                margin: '0 auto',
                padding: '16px 24px',
                display: 'flex',
                alignItems: 'center',
                justifyContent: 'space-between',
            }}>
                {/* Logo */}
                <div style={{ display: 'flex', alignItems: 'center', gap: '8px' }}>
                    <div style={{
                        width: '32px',
                        height: '32px',
                        backgroundColor: '#00f2ea',
                        borderRadius: '8px',
                        display: 'flex',
                        alignItems: 'center',
                        justifyContent: 'center',
                    }}>
                        <span style={{ color: 'black', fontWeight: 'bold', fontSize: '14px' }}>H</span>
                    </div>
                    <span style={{ color: 'white', fontWeight: 600, fontSize: '20px' }}>Hylium</span>
                </div>

                {/* Desktop Navigation */}
                <div style={{
                    display: 'flex',
                    alignItems: 'center',
                    gap: '32px',
                }} className="hidden md:flex">
                    {['Ecosystem', 'Technology', 'Roadmap', 'Whitepaper'].map((item) => (
                        <a
                            key={item}
                            href={`#${item.toLowerCase()}`}
                            style={{
                                color: '#9ca3af',
                                fontSize: '14px',
                                fontWeight: 500,
                                transition: 'color 0.2s',
                            }}
                            onMouseOver={(e) => (e.currentTarget.style.color = 'white')}
                            onMouseOut={(e) => (e.currentTarget.style.color = '#9ca3af')}
                        >
                            {item}
                        </a>
                    ))}
                </div>

                {/* Connect Wallet Button */}
                <button
                    style={{
                        padding: '10px 20px',
                        border: '1px solid rgba(255, 255, 255, 0.3)',
                        borderRadius: '9999px',
                        backgroundColor: 'transparent',
                        color: 'white',
                        fontSize: '14px',
                        fontWeight: 500,
                        cursor: 'pointer',
                        transition: 'all 0.2s',
                    }}
                    onMouseOver={(e) => (e.currentTarget.style.backgroundColor = 'rgba(255, 255, 255, 0.05)')}
                    onMouseOut={(e) => (e.currentTarget.style.backgroundColor = 'transparent')}
                    className="hidden md:block"
                >
                    Connect Wallet
                </button>

                {/* Mobile Menu Button */}
                <button
                    style={{
                        color: 'white',
                        background: 'none',
                        border: 'none',
                        cursor: 'pointer',
                    }}
                    className="md:hidden"
                    onClick={() => setIsOpen(!isOpen)}
                >
                    {isOpen ? <X size={24} /> : <Menu size={24} />}
                </button>
            </div>

            {/* Mobile Menu */}
            {isOpen && (
                <div style={{
                    padding: '16px 24px',
                    borderTop: '1px solid rgba(255, 255, 255, 0.1)',
                    display: 'flex',
                    flexDirection: 'column',
                    gap: '16px',
                }} className="md:hidden">
                    {['Ecosystem', 'Technology', 'Roadmap', 'Whitepaper'].map((item) => (
                        <a
                            key={item}
                            href={`#${item.toLowerCase()}`}
                            style={{ color: '#9ca3af' }}
                        >
                            {item}
                        </a>
                    ))}
                    <button style={{
                        marginTop: '8px',
                        padding: '10px 20px',
                        border: '1px solid rgba(255, 255, 255, 0.3)',
                        borderRadius: '9999px',
                        backgroundColor: 'transparent',
                        color: 'white',
                        fontSize: '14px',
                        fontWeight: 500,
                    }}>
                        Connect Wallet
                    </button>
                </div>
            )}
        </nav>
    );
};

export default Navbar;
