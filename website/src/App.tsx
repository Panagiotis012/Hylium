import Navbar from './components/Navbar';
import Hero from './components/Hero';
import Stats from './components/Stats';
import Features from './components/Features';
import Roadmap from './components/Roadmap';
import Newsletter from './components/Newsletter';
import Footer from './components/Footer';
import './index.css';

function App() {
  return (
    <div style={{ minHeight: '100vh', background: '#f8fafc' }}>
      <Navbar />
      <Hero />
      <Stats />
      <Features />
      <Roadmap />
      <Newsletter />
      <Footer />
    </div>
  );
}

export default App;
