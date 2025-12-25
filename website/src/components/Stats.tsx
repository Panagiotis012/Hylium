
import React, { useEffect, useState } from 'react';
import { getStats } from '../services/api';
import type { NetworkStats } from '../services/api';
import './Stats.css';

const StatItem = ({ label, value, suffix = "" }: { label: string, value: number, suffix?: string }) => {
    const [count, setCount] = useState(0);

    useEffect(() => {
        let start = 0;
        const end = value;
        if (end === 0) {
            setCount(0);
            return;
        }
        const duration = 2000;
        const incrementTime = 20;
        const step = end / (duration / incrementTime);

        const timer = setInterval(() => {
            start += step;
            if (start >= end) {
                setCount(end);
                clearInterval(timer);
            } else {
                setCount(Math.floor(start));
            }
        }, incrementTime);

        return () => clearInterval(timer);
    }, [value]);

    return (
        <div className="stat-item">
            <div className="stat-value">
                {count.toLocaleString()}{suffix}
            </div>
            <div className="stat-label">{label}</div>
        </div>
    );
};

const Stats: React.FC = () => {
    const [stats, setStats] = useState<NetworkStats | null>(null);
    const [loading, setLoading] = useState(true);

    useEffect(() => {
        const fetchStats = async () => {
            const { data } = await getStats();
            if (data) {
                setStats(data);
            }
            setLoading(false);
        };

        fetchStats();
        // Refresh every 30 seconds
        const interval = setInterval(fetchStats, 30000);
        return () => clearInterval(interval);
    }, []);

    if (loading) {
        return (
            <section className="stats-section">
                <div className="stats-container">
                    <div className="stat-item"><span className="stat-loading">Loading...</span></div>
                </div>
            </section>
        );
    }

    return (
        <section className="stats-section">
            <div className="stats-container">
                <StatItem label="Block Height" value={stats?.blockHeight || 0} />
                <StatItem label="Network Hashrate" value={Math.round((stats?.networkHashrate || 0) / 1000000)} suffix=" MH/s" />
                <StatItem label="Difficulty" value={Math.round(stats?.difficulty || 0)} />
                <StatItem label="Connections" value={stats?.connections || 0} />
            </div>
        </section>
    );
};

export default Stats;
