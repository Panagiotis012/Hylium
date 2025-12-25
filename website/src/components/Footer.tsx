
import React from 'react';
import './Footer.css';

const Footer: React.FC = () => {
    return (
        <footer className="footer-section">
            <div className="footer-container">
                <div className="footer-column">
                    <span className="footer-logo">Hylium</span>
                    <p className="footer-copyright">© {new Date().getFullYear()} Hylium. All rights reserved.</p>
                </div>

                <div className="footer-links-group">
                    <h4>Product</h4>
                    <a href="#">Mining</a>
                    <a href="#">Wallet</a>
                    <a href="#">Explorer</a>
                </div>

                <div className="footer-links-group">
                    <h4>Community</h4>
                    <a href="#">Discord</a>
                    <a href="#">Twitter</a>
                    <a href="#">GitHub</a>
                </div>

                <div className="footer-links-group">
                    <h4>Resources</h4>
                    <a href="#">Whitepaper</a>
                    <a href="#">Documentation</a>
                    <a href="#">Status</a>
                </div>
            </div>
        </footer>
    );
};

export default Footer;
