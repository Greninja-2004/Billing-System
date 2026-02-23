import { useNavigate } from 'react-router-dom';
import { Button } from '../components/ui/button';
import { Card, CardContent, CardHeader, CardTitle, CardDescription, CardFooter } from '../components/ui/card';
import { Badge } from '../components/ui/badge';
import { Check, Shield, Zap, BarChart3, Github, Apple } from 'lucide-react';
import { useAuthStore } from '../store/auth';

export const LandingPage = () => {
    const navigate = useNavigate();
    const { isAuthenticated } = useAuthStore();

    return (
        <div className="min-h-screen bg-background text-foreground selection:bg-primary/30">
            {/* Header / Navbar */}
            <header className="fixed top-0 w-full border-b border-white/5 bg-background/80 backdrop-blur-md z-50">
                <div className="max-w-7xl mx-auto px-6 h-16 flex items-center justify-between">
                    <div className="flex items-center space-x-2 font-bold text-xl tracking-tight">
                        <div className="h-8 w-8 rounded-lg bg-primary flex items-center justify-center text-primary-foreground text-sm">BS</div>
                        <span>Billing System<span className="text-primary">.pro</span></span>
                    </div>
                    <nav className="hidden md:flex space-x-8 text-sm font-medium text-muted-foreground">
                        <a href="#features" className="hover:text-foreground transition-colors">Features</a>
                        <a href="#pricing" className="hover:text-foreground transition-colors">Pricing</a>
                        <a href="#developers" className="hover:text-foreground transition-colors">Developers</a>
                    </nav>
                    <div className="flex items-center space-x-4">
                        {isAuthenticated ? (
                            <Button className="hover-lift rounded-full px-6" onClick={() => navigate('/dashboard')}>
                                Go to Dashboard
                            </Button>
                        ) : (
                            <>
                                <Button variant="ghost" onClick={() => navigate('/login')} className="hidden sm:inline-flex">Sign In</Button>
                                <Button className="hover-lift rounded-full px-6" onClick={() => navigate('/login')}>Get Started</Button>
                            </>
                        )}
                    </div>
                </div>
            </header>

            <main className="pt-24 pb-16">
                {/* Hero Section */}
                <section className="relative max-w-7xl mx-auto px-6 pt-20 pb-32 text-center">
                    <div className="absolute top-1/2 left-1/2 -translate-x-1/2 -translate-y-1/2 w-[600px] h-[600px] bg-primary/20 rounded-full blur-[120px] -z-10 pointer-events-none" />
                    <Badge variant="outline" className="mb-6 px-4 py-1.5 border-primary/30 text-primary bg-primary/5 rounded-full text-sm font-medium tracking-wide">
                        Billing Engine v2.0 is Live
                    </Badge>
                    <h1 className="text-5xl md:text-7xl font-extrabold tracking-tight mb-8 leading-tight">
                        Financial Infrastructure <br className="hidden md:block" />
                        <span className="text-transparent bg-clip-text bg-gradient-to-r from-primary to-blue-400">for the Internet.</span>
                    </h1>
                    <p className="text-xl text-muted-foreground max-w-2xl mx-auto mb-10 leading-relaxed">
                        A fully integrated suite of payment products. Accept payments, manage subscriptions, and automate your financial operations globally.
                    </p>
                    <div className="flex flex-col sm:flex-row items-center justify-center gap-4">
                        <Button size="lg" className="h-14 px-8 text-lg rounded-full hover-lift" onClick={() => navigate('/login')}>
                            Start building for free
                        </Button>
                        <Button size="lg" variant="outline" className="h-14 px-8 text-lg rounded-full hover-lift bg-background/50 backdrop-blur">
                            Contact Sales
                        </Button>
                    </div>

                    {/* Social Auth Preview in Hero */}
                    <div className="mt-16 pt-8 border-t border-border/50 max-w-md mx-auto">
                        <p className="text-sm text-muted-foreground mb-6 font-medium">Or continue with</p>
                        <div className="grid grid-cols-3 gap-4">
                            <Button variant="outline" className="w-full flex items-center justify-center hover-lift h-12" onClick={() => navigate('/login')}>
                                <Github className="h-5 w-5" />
                            </Button>
                            <Button variant="outline" className="w-full flex items-center justify-center hover-lift h-12" onClick={() => navigate('/login')}>
                                <Apple className="h-5 w-5 fill-current" />
                            </Button>
                            <Button variant="outline" className="w-full flex items-center justify-center hover-lift h-12" onClick={() => navigate('/login')}>
                                <svg className="h-5 w-5" viewBox="0 0 24 24">
                                    <path d="M22.56 12.25c0-.78-.07-1.53-.2-2.25H12v4.26h5.92c-.26 1.37-1.04 2.53-2.21 3.31v2.77h3.57c2.08-1.92 3.28-4.74 3.28-8.09z" fill="#4285F4" />
                                    <path d="M12 23c2.97 0 5.46-.98 7.28-2.66l-3.57-2.77c-.98.66-2.23 1.06-3.71 1.06-2.86 0-5.29-1.93-6.16-4.53H2.18v2.84C3.99 20.53 7.7 23 12 23z" fill="#34A853" />
                                    <path d="M5.84 14.09c-.22-.66-.35-1.36-.35-2.09s.13-1.43.35-2.09V7.07H2.18C1.43 8.55 1 10.22 1 12s.43 3.45 1.18 4.93l2.85-2.22.81-.62z" fill="#FBBC05" />
                                    <path d="M12 5.38c1.62 0 3.06.56 4.21 1.64l3.15-3.15C17.45 2.09 14.97 1 12 1 7.7 1 3.99 3.47 2.18 7.07l3.66 2.84c.87-2.6 3.3-4.53 6.16-4.53z" fill="#EA4335" />
                                </svg>
                            </Button>
                        </div>
                    </div>
                </section>

                {/* Features Section */}
                <section id="features" className="py-24 bg-muted/20 border-y border-border/50">
                    <div className="max-w-7xl mx-auto px-6">
                        <div className="text-center mb-16">
                            <h2 className="text-3xl font-bold tracking-tight mb-4">Everything you need to scale</h2>
                            <p className="text-muted-foreground max-w-2xl mx-auto text-lg">
                                Powerful APIs, a beautiful dashboard, and deep analytics. Built for modern software companies.
                            </p>
                        </div>
                        <div className="grid md:grid-cols-3 gap-8">
                            <Card className="bg-background/50 border-white/5 hover-lift backdrop-blur-sm">
                                <CardHeader>
                                    <div className="h-12 w-12 rounded-lg bg-blue-500/10 text-blue-500 flex items-center justify-center mb-4">
                                        <Shield className="h-6 w-6" />
                                    </div>
                                    <CardTitle>Global Payments</CardTitle>
                                </CardHeader>
                                <CardContent className="text-muted-foreground leading-relaxed">
                                    Accept credit cards, wallets, and local payment methods around the world. Secure, PCI-compliant infrastructure out of the box.
                                </CardContent>
                            </Card>
                            <Card className="bg-background/50 border-white/5 hover-lift backdrop-blur-sm">
                                <CardHeader>
                                    <div className="h-12 w-12 rounded-lg bg-emerald-500/10 text-emerald-500 flex items-center justify-center mb-4">
                                        <Zap className="h-6 w-6" />
                                    </div>
                                    <CardTitle>Smart Subscriptions</CardTitle>
                                </CardHeader>
                                <CardContent className="text-muted-foreground leading-relaxed">
                                    Handle complex billing logic. Prorations, usage-based billing, grandfathering, and automated dunning/retries are fully supported.
                                </CardContent>
                            </Card>
                            <Card className="bg-background/50 border-white/5 hover-lift backdrop-blur-sm">
                                <CardHeader>
                                    <div className="h-12 w-12 rounded-lg bg-purple-500/10 text-purple-500 flex items-center justify-center mb-4">
                                        <BarChart3 className="h-6 w-6" />
                                    </div>
                                    <CardTitle>Financial Analytics</CardTitle>
                                </CardHeader>
                                <CardContent className="text-muted-foreground leading-relaxed">
                                    Real-time reporting on MRR, churn, lifetime value, and accounts receivable aging. Automatically generated investor-ready reports.
                                </CardContent>
                            </Card>
                        </div>
                    </div>
                </section>

                {/* Pricing Section */}
                <section id="pricing" className="py-32">
                    <div className="max-w-7xl mx-auto px-6">
                        <div className="text-center mb-20">
                            <h2 className="text-4xl font-extrabold tracking-tight mb-4">Simple, transparent pricing</h2>
                            <p className="text-xl text-muted-foreground max-w-2xl mx-auto">
                                Always know what you'll pay. No hidden fees or surprise charges.
                            </p>
                        </div>
                        <div className="grid md:grid-cols-3 gap-8 max-w-5xl mx-auto">
                            {/* Starter Plan */}
                            <Card className="flex flex-col hover-lift border-border bg-background">
                                <CardHeader>
                                    <CardTitle className="text-2xl">Developer</CardTitle>
                                    <CardDescription>Perfect for side projects and startups.</CardDescription>
                                    <div className="mt-4 flex items-baseline text-5xl font-extrabold">
                                        $0
                                        <span className="ml-1 text-xl font-medium text-muted-foreground">/mo</span>
                                    </div>
                                </CardHeader>
                                <CardContent className="flex-1">
                                    <ul className="space-y-4 mt-4">
                                        <li className="flex items-center"><Check className="h-5 w-5 text-emerald-500 mr-3 shrink-0" /> Pay-as-you-go processing (2.9% + 30¢)</li>
                                        <li className="flex items-center"><Check className="h-5 w-5 text-emerald-500 mr-3 shrink-0" /> Basic invoicing</li>
                                        <li className="flex items-center"><Check className="h-5 w-5 text-emerald-500 mr-3 shrink-0" /> Standard analytics</li>
                                        <li className="flex items-center"><Check className="h-5 w-5 text-emerald-500 mr-3 shrink-0" /> Email support</li>
                                    </ul>
                                </CardContent>
                                <CardFooter>
                                    <Button className="w-full hover-lift h-12 text-md" variant="outline" onClick={() => navigate('/login')}>Get Started</Button>
                                </CardFooter>
                            </Card>

                            {/* Pro Plan */}
                            <Card className="flex flex-col hover-lift border-primary shadow-lg shadow-primary/10 relative transform md:-translate-y-4">
                                <div className="absolute top-0 inset-x-0 h-1 bg-primary rounded-t-xl" />
                                <Badge className="absolute -top-3 left-1/2 -translate-x-1/2 bg-primary text-primary-foreground px-3 py-1 uppercase tracking-widest text-xs font-bold shadow-md">
                                    Most Popular
                                </Badge>
                                <CardHeader className="pt-8">
                                    <CardTitle className="text-2xl text-foreground">Growth</CardTitle>
                                    <CardDescription>Advanced features for scaling businesses.</CardDescription>
                                    <div className="mt-4 flex items-baseline text-5xl font-extrabold">
                                        $99
                                        <span className="ml-1 text-xl font-medium text-muted-foreground">/mo</span>
                                    </div>
                                </CardHeader>
                                <CardContent className="flex-1">
                                    <ul className="space-y-4 mt-4 text-foreground">
                                        <li className="flex items-center"><Check className="h-5 w-5 text-primary mr-3 shrink-0" /> Discounted processing (2.5% + 30¢)</li>
                                        <li className="flex items-center"><Check className="h-5 w-5 text-primary mr-3 shrink-0" /> Advanced subscription logic</li>
                                        <li className="flex items-center"><Check className="h-5 w-5 text-primary mr-3 shrink-0" /> Webhooks & custom integrations</li>
                                        <li className="flex items-center"><Check className="h-5 w-5 text-primary mr-3 shrink-0" /> Multi-currency support</li>
                                        <li className="flex items-center"><Check className="h-5 w-5 text-primary mr-3 shrink-0" /> Priority 24/7 support</li>
                                    </ul>
                                </CardContent>
                                <CardFooter>
                                    <Button className="w-full hover-lift h-12 text-md shadow-md shadow-primary/20" onClick={() => navigate('/login')}>Start Free Trial</Button>
                                </CardFooter>
                            </Card>

                            {/* Enterprise Plan */}
                            <Card className="flex flex-col hover-lift border-border bg-background">
                                <CardHeader>
                                    <CardTitle className="text-2xl">Enterprise</CardTitle>
                                    <CardDescription>Custom solutions for large volumes.</CardDescription>
                                    <div className="mt-4 flex items-baseline text-5xl font-extrabold">
                                        Custom
                                    </div>
                                </CardHeader>
                                <CardContent className="flex-1">
                                    <ul className="space-y-4 mt-4">
                                        <li className="flex items-center"><Check className="h-5 w-5 text-emerald-500 mr-3 shrink-0" /> Volume discounts</li>
                                        <li className="flex items-center"><Check className="h-5 w-5 text-emerald-500 mr-3 shrink-0" /> Dedicated account manager</li>
                                        <li className="flex items-center"><Check className="h-5 w-5 text-emerald-500 mr-3 shrink-0" /> SLA guarantees (99.99%)</li>
                                        <li className="flex items-center"><Check className="h-5 w-5 text-emerald-500 mr-3 shrink-0" /> Custom legal & security terms</li>
                                    </ul>
                                </CardContent>
                                <CardFooter>
                                    <Button className="w-full hover-lift h-12 text-md" variant="outline" onClick={() => alert('Opening Contact Form...')}>Contact Us</Button>
                                </CardFooter>
                            </Card>
                        </div>
                    </div>
                </section>
            </main>

            {/* Footer */}
            <footer className="border-t border-border/50 bg-muted/10 py-12 text-center text-sm text-muted-foreground">
                <div className="max-w-7xl mx-auto px-6 grid grid-cols-2 md:grid-cols-4 gap-8 mb-12 text-left">
                    <div>
                        <h4 className="font-semibold text-foreground mb-4">Products</h4>
                        <ul className="space-y-2">
                            <li><a href="#" className="hover:text-primary transition-colors">Payments</a></li>
                            <li><a href="#" className="hover:text-primary transition-colors">Billing</a></li>
                            <li><a href="#" className="hover:text-primary transition-colors">Invoicing</a></li>
                        </ul>
                    </div>
                    <div>
                        <h4 className="font-semibold text-foreground mb-4">Developers</h4>
                        <ul className="space-y-2">
                            <li><a href="#" className="hover:text-primary transition-colors">Documentation</a></li>
                            <li><a href="#" className="hover:text-primary transition-colors">API Reference</a></li>
                            <li><a href="#" className="hover:text-primary transition-colors">Status</a></li>
                        </ul>
                    </div>
                    <div>
                        <h4 className="font-semibold text-foreground mb-4">Company</h4>
                        <ul className="space-y-2">
                            <li><a href="#" className="hover:text-primary transition-colors">About</a></li>
                            <li><a href="#" className="hover:text-primary transition-colors">Customers</a></li>
                            <li><a href="#" className="hover:text-primary transition-colors">Careers</a></li>
                        </ul>
                    </div>
                    <div>
                        <h4 className="font-semibold text-foreground mb-4">Legal</h4>
                        <ul className="space-y-2">
                            <li><a href="#" className="hover:text-primary transition-colors">Privacy</a></li>
                            <li><a href="#" className="hover:text-primary transition-colors">Terms</a></li>
                        </ul>
                    </div>
                </div>
                <p>&copy; 2026 Billing System Pro. All rights reserved.</p>
            </footer>
        </div>
    );
};
