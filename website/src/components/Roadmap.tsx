
import React from 'react';
import { motion } from 'framer-motion';
import './Roadmap.css';

const roadmapData = [
    {
        phase: "Phase 1",
        title: "Foundation",
        date: "Q3 2024",
        items: ["Core Protocol Launch", "Wallet Beta", "Miner V1 Release"]
    },
    {
        phase: "Phase 2",
        title: "Expansion",
        date: "Q4 2024",
        items: ["Exchange Listings", "Smart Contracts Alpha", "Mobile Wallet"]
    },
    {
        phase: "Phase 3",
        title: "Ecosystem",
        date: "Q1 2025",
        items: ["DeFi Bridge", "Governance DAO", "Global Hackathon"]
    },
    {
        phase: "Phase 4",
        title: "Sovereignty",
        date: "Q3 2025",
        items: ["Full Decentralization", "Layer 2 Scaling", "Privacy Features"]
    }
];

const Roadmap: React.FC = () => {
    return (
        <section className="roadmap-section" id="roadmap">
            <h2 className="section-title center">Roadmap</h2>
            <div className="roadmap-container">
                <div className="roadmap-line"></div>
                {roadmapData.map((item, index) => (
                    <motion.div
                        className={`roadmap-item ${index % 2 === 0 ? 'left' : 'right'}`}
                        key={index}
                        initial={{ opacity: 0, x: index % 2 === 0 ? -50 : 50 }}
                        whileInView={{ opacity: 1, x: 0 }}
                        viewport={{ once: true, margin: "-100px" }}
                        transition={{ duration: 0.6, delay: index * 0.1 }}
                    >
                        <div className="roadmap-content">
                            <span className="roadmap-date">{item.date}</span>
                            <h3 className="roadmap-phase">{item.phase}: {item.title}</h3>
                            <ul className="roadmap-list">
                                {item.items.map((subItem, i) => (
                                    <li key={i}>{subItem}</li>
                                ))}
                            </ul>
                        </div>
                        <div className="roadmap-dot"></div>
                    </motion.div>
                ))}
            </div>
        </section>
    );
};

export default Roadmap;
