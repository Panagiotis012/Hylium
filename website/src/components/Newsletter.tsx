
import React, { useState } from 'react';
import { subscribeNewsletter } from '../services/api';
import './Newsletter.css';

const Newsletter: React.FC = () => {
    const [email, setEmail] = useState('');
    const [status, setStatus] = useState<'idle' | 'loading' | 'success' | 'error'>('idle');
    const [message, setMessage] = useState('');

    const handleSubmit = async (e: React.FormEvent) => {
        e.preventDefault();
        if (!email) return;

        setStatus('loading');
        const { data, error } = await subscribeNewsletter(email);

        if (error) {
            setStatus('error');
            setMessage(error);
        } else {
            setStatus('success');
            setMessage(data?.message || 'Subscribed!');
            setEmail('');
        }

        // Reset status after 3 seconds
        setTimeout(() => {
            setStatus('idle');
            setMessage('');
        }, 3000);
    };

    return (
        <section className="newsletter-section">
            <div className="newsletter-container">
                <h2 className="newsletter-title">Stay Updated</h2>
                <p className="newsletter-subtitle">Get the latest news about Hylium development</p>

                <form className="newsletter-form" onSubmit={handleSubmit}>
                    <input
                        type="email"
                        placeholder="Enter your email"
                        value={email}
                        onChange={(e) => setEmail(e.target.value)}
                        className="newsletter-input"
                        disabled={status === 'loading'}
                        required
                    />
                    <button
                        type="submit"
                        className="newsletter-btn"
                        disabled={status === 'loading'}
                    >
                        {status === 'loading' ? 'Subscribing...' : 'Subscribe'}
                    </button>
                </form>

                {message && (
                    <p className={`newsletter-message ${status}`}>
                        {message}
                    </p>
                )}
            </div>
        </section>
    );
};

export default Newsletter;
