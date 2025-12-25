import Navbar from './components/Navbar';
import Hero from './components/Hero';
import TrustFooter from './components/TrustFooter';
import './index.css';

function App() {
  return (
    <div className="min-h-screen bg-gradient-to-br from-[#0a0a0c] via-[#0d0d12] to-[#0a0a14]">
      <Navbar />
      <Hero />
      <TrustFooter />
    </div>
  );
}

export default App;
