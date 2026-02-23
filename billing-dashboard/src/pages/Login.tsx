import { API_BASE_URL } from "../config";
import { useState, useEffect } from 'react';
import { useAuthStore } from '../store/auth';
import { useNavigate, useLocation } from 'react-router-dom';
import { Github, Eye, EyeOff, Asterisk } from 'lucide-react';
import { Button } from '../components/ui/button';
import { Input } from '../components/ui/input';

export const Login = () => {
    const [isSignUp, setIsSignUp] = useState(false);
    const [email, setEmail] = useState('');
    const [username, setUsername] = useState('admin');
    const [password, setPassword] = useState('admin123');
    const [showPassword, setShowPassword] = useState(false);
    const [error, setError] = useState(false);
    const login = useAuthStore((state) => state.login);
    const navigate = useNavigate();
    const location = useLocation();

    useEffect(() => {
        const params = new URLSearchParams(location.search);
        const socialToken = params.get('social_token');
        const socialUser = params.get('social_user');
        const err = params.get('error');

        if (err) {
            setError(true);
            setTimeout(() => setError(false), 2000);
            return;
        }

        if (socialToken && socialUser) {
            try {
                const userObj = JSON.parse(decodeURIComponent(socialUser));
                login(userObj, socialToken);
                navigate('/dashboard', { replace: true });
            } catch (e) {
                console.error("Failed to parse social user details", e);
            }
        }
    }, [location, login, navigate]);

    const handleAuth = async (e: React.FormEvent) => {
        e.preventDefault();

        try {
            const endpoint = isSignUp ? `${API_BASE_URL}/api/signup` : `${API_BASE_URL}/api/login`;
            const payload = isSignUp ? { username, email, password } : { username, password };

            const res = await fetch(endpoint, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(payload)
            });
            const data = await res.json();

            if (res.ok && data.success) {
                login(data.user, data.token);
                setError(false);
                navigate('/dashboard');
            } else {
                throw new Error(data.message || `${isSignUp ? 'Signup' : 'Login'} failed`);
            }
        } catch (err) {
            console.error(err);
            setError(true);
            setTimeout(() => setError(false), 500); // Remove shake class after animation
        }
    };

    const handleSocialLogin = (provider: string) => {
        // Redirect to exact backend route for OAuth flow
        window.location.href = `${API_BASE_URL}/api/auth/${provider}`;
    };

    return (
        <div className="min-h-screen flex items-center justify-center bg-background p-4 sm:p-8">
            <div className={`w-full max-w-[380px] space-y-8 ${error ? 'animate-shake' : ''}`}>
                <div className="space-y-6 text-left">
                    <div className="h-12 w-12 rounded-xl bg-foreground flex items-center justify-center text-background">
                        <Asterisk className="h-8 w-8" />
                    </div>
                    <div className="space-y-2">
                        <h1 className="text-[28px] font-semibold tracking-tight text-foreground">
                            {isSignUp ? "Create an Account" : "Login to Billing Pro"}
                        </h1>
                        <p className="text-sm text-muted-foreground leading-relaxed">
                            {isSignUp ? "Join us and boost your business workflow" : "Manage your financial operations and boost your business workflow"}
                        </p>
                    </div>
                </div>

                <form onSubmit={handleAuth} className="space-y-4">
                    <div className="space-y-4">
                        {isSignUp && (
                            <Input
                                id="email"
                                placeholder="Email address"
                                type="email"
                                value={email}
                                onChange={(e) => setEmail(e.target.value)}
                                className="h-12 rounded-xl bg-transparent border-input"
                                required
                            />
                        )}
                        <Input
                            id="username"
                            placeholder="Email or username"
                            autoCapitalize="none"
                            autoComplete="username"
                            value={username}
                            onChange={(e) => setUsername(e.target.value)}
                            className="h-12 rounded-xl bg-transparent border-input"
                            required
                        />
                        <div className="relative">
                            <Input
                                id="password"
                                placeholder="Password"
                                type={showPassword ? "text" : "password"}
                                autoComplete="current-password"
                                value={password}
                                onChange={(e) => setPassword(e.target.value)}
                                className="h-12 rounded-xl bg-transparent border-input pr-10"
                                required
                            />
                            <button
                                type="button"
                                onClick={() => setShowPassword(!showPassword)}
                                className="absolute right-4 top-1/2 -translate-y-1/2 text-muted-foreground hover:text-foreground transition-colors"
                            >
                                {showPassword ? <EyeOff className="h-5 w-5" /> : <Eye className="h-5 w-5" />}
                            </button>
                        </div>
                    </div>

                    {error && (
                        <p className="text-sm text-destructive font-medium">
                            {isSignUp ? 'Username or email may already exist' : 'Invalid username or password'}
                        </p>
                    )}

                    <Button type="submit" className="w-full h-12 rounded-xl bg-[#c5f82a] hover:bg-[#bdf241] text-black font-semibold text-sm transition-colors shadow-none border-0">
                        {isSignUp ? 'Sign up' : 'Log in'}
                    </Button>
                </form>

                <div className="relative pt-2">
                    <div className="absolute inset-0 flex items-center pt-2">
                        <span className="w-full border-t border-border" />
                    </div>
                    <div className="relative flex justify-center text-xs">
                        <span className="bg-background px-4 text-muted-foreground">Or authorize with</span>
                    </div>
                </div>

                <div className="grid grid-cols-3 gap-3">
                    <Button variant="outline" className="w-full h-12 rounded-xl bg-transparent" onClick={() => handleSocialLogin('google')}>
                        <svg className="h-5 w-5" viewBox="0 0 24 24">
                            <path d="M22.56 12.25c0-.78-.07-1.53-.2-2.25H12v4.26h5.92c-.26 1.37-1.04 2.53-2.21 3.31v2.77h3.57c2.08-1.92 3.28-4.74 3.28-8.09z" fill="#4285F4" />
                            <path d="M12 23c2.97 0 5.46-.98 7.28-2.66l-3.57-2.77c-.98.66-2.23 1.06-3.71 1.06-2.86 0-5.29-1.93-6.16-4.53H2.18v2.84C3.99 20.53 7.7 23 12 23z" fill="#34A853" />
                            <path d="M5.84 14.09c-.22-.66-.35-1.36-.35-2.09s.13-1.43.35-2.09V7.07H2.18C1.43 8.55 1 10.22 1 12s.43 3.45 1.18 4.93l2.85-2.22.81-.62z" fill="#FBBC05" />
                            <path d="M12 5.38c1.62 0 3.06.56 4.21 1.64l3.15-3.15C17.45 2.09 14.97 1 12 1 7.7 1 3.99 3.47 2.18 7.07l3.66 2.84c.87-2.6 3.3-4.53 6.16-4.53z" fill="#EA4335" />
                        </svg>
                    </Button>
                    <Button variant="outline" className="w-full h-12 rounded-xl bg-transparent" onClick={() => handleSocialLogin('github')}>
                        <Github className="h-5 w-5" />
                    </Button>
                    <Button variant="outline" className="w-full h-12 rounded-xl bg-transparent" onClick={() => handleSocialLogin('apple')}>
                        <svg className="h-5 w-5 fill-current" viewBox="0 0 384 512">
                            <path d="M318.7 268.7c-.2-36.7 16.4-64.4 50-84.8-18.8-26.9-47.2-41.7-84.7-44.6-35.5-2.8-74.3 20.7-88.5 20.7-15 0-49.4-19.7-76.4-19.7C63.3 141.2 4 184.8 4 273.5q0 39.3 14.4 81.2c12.8 36.7 59 126.7 107.2 125.2 25.2-.6 43-17.9 75.8-17.9 31.8 0 48.3 17.9 76.4 17.9 48.6-.7 90.4-82.5 102.6-119.3-65.2-30.7-61.7-90-61.7-91.9zm-56.6-164.2c27.3-32.4 24.8-61.9 24-72.5-24.1 1.4-52 16.4-67.9 34.9-17.5 19.8-27.8 44.3-25.6 71.9 26.1 2 49.9-11.4 69.5-34.3z" />
                        </svg>
                    </Button>
                </div>

                <div className="space-y-3 pt-4 text-[13px]">
                    {!isSignUp && (
                        <a href="#" onClick={(e) => e.preventDefault()} className="font-medium hover:underline block text-foreground">
                            Forgot password?
                        </a>
                    )}
                    <div className="text-muted-foreground">
                        {isSignUp ? 'Already have an account? ' : "Don't have an account? "}
                        <a href="#" onClick={(e) => { e.preventDefault(); setIsSignUp(!isSignUp); }} className="text-foreground font-medium hover:underline">
                            {isSignUp ? 'Log in' : 'Sign up'}
                        </a>
                    </div>
                </div>
            </div>
        </div>
    );
};
