import { useState } from 'react';
import { subscribeNewsletter } from '../services/api';

const Newsletter = () => {
    const [email, setEmail] = useState('');
    const [status, setStatus] = useState<'idle' | 'loading' | 'success' | 'error'>('idle');
    const [msg, setMsg] = useState('');

    const handleSubmit = async (e: React.FormEvent) => {
        e.preventDefault();
        if (!email) return;
        setStatus('loading');
        const { data, error } = await subscribeNewsletter(email);
        if (error) {
            setStatus('error');
            setMsg(error);
        } else {
            setStatus('success');
            setMsg(data?.message || 'Subscribed!');
            setEmail('');
        }
        setTimeout(() => { setStatus('idle'); setMsg(''); }, 3000);
    };

    return (
        <section style={{ padding: '80px 24px', background: '#ffffff' }}>
            <div style={{ maxWidth: '600px', margin: '0 auto', textAlign: 'center' }}>
                <h2 style={{ fontSize: '28px', fontWeight: 700, color: '#0f172a', marginBottom: '12px' }}>
                    Stay Updated
                </h2>
                <p style={{ color: '#64748b', marginBottom: '32px' }}>
                    Get the latest Hylium news and updates
                </p>
                <form onSubmit={handleSubmit} style={{ display: 'flex', gap: '12px', justifyContent: 'center' }}>
                    <input
                        type="email"
                        placeholder="Enter your email"
                        value={email}
                        onChange={(e) => setEmail(e.target.value)}
                        style={{
                            flex: 1,
                            maxWidth: '320px',
                            padding: '14px 20px',
                            borderRadius: '12px',
                            border: '1px solid #e2e8f0',
                            fontSize: '15px',
                            outline: 'none',
                        }}
                        disabled={status === 'loading'}
                        required
                    />
                    <button type="submit" disabled={status === 'loading'} style={{
                        padding: '14px 28px',
                        borderRadius: '12px',
                        backgroundColor: '#00d4ff',
                        color: '#0f172a',
                        fontWeight: 600,
                        border: 'none',
                        cursor: 'pointer',
                        fontSize: '15px',
                    }}>
                        {status === 'loading' ? 'Subscribing...' : 'Subscribe'}
                    </button>
                </form>
                {msg && <p style={{ marginTop: '16px', color: status === 'success' ? '#10b981' : '#ef4444' }}>{msg}</p>}
            </div>
        </section>
    );
};

export default Newsletter;
