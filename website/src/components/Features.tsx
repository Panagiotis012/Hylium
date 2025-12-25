
import React from 'react';
import { Shield, Zap, Globe, Cpu } from 'lucide-react';
import './Features.css';

const featuresData = [
    {
        icon: <Shield size={32} />,
        title: "Secure by Design",
        description: "Built on advanced cryptographic principles ensuring your assets remain safe at all times."
    },
    {
        icon: <Zap size={32} />,
        title: "Lightning Fast",
        description: "Experience transaction speeds that rival centralized systems without compromising decentralization."
    },
    {
        icon: <Globe size={32} />,
        title: "Global Network",
        description: "Join a distributed network of miners and validators across over 100 countries."
    },
    {
        icon: <Cpu size={32} />,
        title: "Efficient Mining",
        description: "Optimized algorithms allow for mining on consumer hardware with minimal energy waste."
    }
];

const Features: React.FC = () => {
    return (
        <section className="features-section" id="features">
            <div className="features-container">
                <div className="section-header">
                    <h2 className="section-title">Key Features</h2>
                    <p className="section-subtitle">Why Hylium is the choice for the next generation.</p>
                </div>

                <div className="features-grid">
                    {featuresData.map((feature, index) => (
                        <div className="feature-card" key={index}>
                            <div className="feature-icon">{feature.icon}</div>
                            <h3 className="feature-title">{feature.title}</h3>
                            <p className="feature-description">{feature.description}</p>
                        </div>
                    ))}
                </div>
            </div>
        </section>
    );
};

export default Features;
