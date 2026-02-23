import { Outlet, NavLink, useNavigate } from 'react-router-dom';
import { useAuthStore } from '../store/auth';
import {
    LayoutDashboard, Users, FileText, CreditCard, BarChart3,
    Bell, LogOut, Settings, ShieldAlert, Moon, Sun, Search,
    ChevronDown, Activity, Home
} from 'lucide-react';
import { Button } from '../components/ui/button';
import { useEffect, useState } from 'react';

const NAV_ITEMS = [
    { name: 'Dashboard', path: '/dashboard', icon: LayoutDashboard, roles: ['ADMIN', 'MANAGER', 'VIEWER'] },
    { name: 'Customers', path: '/customers', icon: Users, roles: ['ADMIN', 'MANAGER', 'VIEWER'] },
    { name: 'Invoices', path: '/invoices', icon: FileText, roles: ['ADMIN', 'MANAGER', 'VIEWER'] },
    { name: 'Payments', path: '/payments', icon: CreditCard, roles: ['ADMIN', 'MANAGER', 'VIEWER'] },
    { name: 'Reports', path: '/reports', icon: BarChart3, roles: ['ADMIN', 'MANAGER', 'VIEWER'] },
    { name: 'Notifications', path: '/notifications', icon: Bell, roles: ['ADMIN', 'MANAGER', 'VIEWER'], badge: 3 },
    { name: 'Users', path: '/users', icon: ShieldAlert, roles: ['ADMIN'] },
    { name: 'Audit Log', path: '/audit', icon: Activity, roles: ['ADMIN', 'MANAGER'] },
    { name: 'Settings', path: '/settings', icon: Settings, roles: ['ADMIN', 'MANAGER', 'VIEWER'] },
];

const ROLE_COLORS: Record<string, string> = {
    ADMIN: 'bg-red-500/10 text-red-600',
    MANAGER: 'bg-amber-500/10 text-amber-600',
    VIEWER: 'bg-blue-500/10 text-blue-600',
};

export const MainLayout = () => {
    const { user, logout } = useAuthStore();
    const navigate = useNavigate();
    const [theme, setTheme] = useState<'light' | 'dark'>(
        (localStorage.getItem('theme') as 'light' | 'dark') || 'dark'
    );
    const [searchQuery, setSearchQuery] = useState('');
    const [showUserMenu, setShowUserMenu] = useState(false);

    useEffect(() => {
        const root = window.document.documentElement;
        root.classList.remove('light', 'dark');
        root.classList.add(theme);
        localStorage.setItem('theme', theme);
    }, [theme]);

    const handleLogout = () => {
        logout();
        navigate('/login');
    };

    const filteredNav = NAV_ITEMS.filter(item => user && item.roles.includes(user.role));
    const userInitial = (user?.username || '?').charAt(0).toUpperCase();

    // Close menu on outside click
    useEffect(() => {
        const handler = (e: MouseEvent) => {
            const target = e.target as HTMLElement;
            if (!target.closest('[data-user-menu]')) setShowUserMenu(false);
        };
        document.addEventListener('click', handler);
        return () => document.removeEventListener('click', handler);
    }, []);

    return (
        <div className="flex h-screen overflow-hidden bg-background">

            {/* ── Sidebar ── */}
            <aside className="w-64 border-r bg-card flex-shrink-0 flex flex-col">

                {/* Logo */}
                <div className="h-16 flex items-center px-5 border-b flex-shrink-0">
                    <div className="flex items-center gap-3">
                        <div className="h-9 w-9 rounded-xl bg-primary flex items-center justify-center text-primary-foreground font-bold text-sm shadow-md shadow-primary/30 flex-shrink-0">
                            BP
                        </div>
                        <div>
                            <span className="text-base font-bold tracking-tight block">Billing Pro</span>
                            <span className="text-[10px] text-muted-foreground font-medium">Financial Dashboard</span>
                        </div>
                    </div>
                </div>

                {/* Navigation */}
                <nav className="flex-1 overflow-y-auto py-4 px-3 space-y-0.5">
                    <div className="mb-3 px-3">
                        <p className="text-[10px] font-semibold text-muted-foreground uppercase tracking-widest">Main Menu</p>
                    </div>

                    {filteredNav.map((item) => {
                        const Icon = item.icon;
                        return (
                            <NavLink
                                key={item.path}
                                to={item.path}
                                className={({ isActive }) =>
                                    `flex items-center px-3 py-2.5 text-sm font-medium rounded-xl transition-all duration-200 group relative ${isActive
                                        ? 'bg-primary text-primary-foreground shadow-md shadow-primary/25'
                                        : 'text-muted-foreground hover:bg-muted hover:text-foreground'
                                    }`
                                }
                            >
                                {({ isActive }) => (
                                    <>
                                        <Icon
                                            size={18}
                                            className={`mr-3 flex-shrink-0 transition-transform duration-200 group-hover:scale-110 ${isActive ? '' : ''}`}
                                        />
                                        <span className="flex-1 truncate">{item.name}</span>
                                        {item.badge ? (
                                            <span className={`text-[10px] font-bold px-1.5 py-0.5 rounded-full min-w-[18px] text-center ${isActive ? 'bg-white/20 text-white' : 'bg-red-500 text-white'
                                                }`}>
                                                {item.badge}
                                            </span>
                                        ) : null}
                                    </>
                                )}
                            </NavLink>
                        );
                    })}
                </nav>

                {/* User Footer */}
                <div className="p-3 border-t flex-shrink-0">
                    <div
                        data-user-menu
                        onClick={() => setShowUserMenu(v => !v)}
                        className="flex items-center gap-3 p-2.5 rounded-xl hover:bg-muted cursor-pointer transition-colors relative"
                    >
                        <div className="h-9 w-9 rounded-full bg-primary/20 flex items-center justify-center text-primary font-bold text-sm flex-shrink-0">
                            {userInitial}
                        </div>
                        <div className="flex-1 min-w-0">
                            <p className="text-sm font-semibold truncate">{user?.username}</p>
                            <p className={`text-[10px] font-medium px-1.5 py-0.5 rounded-full inline-block mt-0.5 ${ROLE_COLORS[user?.role || ''] ?? 'bg-muted text-muted-foreground'}`}>
                                {user?.role}
                            </p>
                        </div>
                        <ChevronDown size={14} className={`text-muted-foreground transition-transform ${showUserMenu ? 'rotate-180' : ''}`} />

                        {/* Dropdown */}
                        {showUserMenu && (
                            <div className="absolute bottom-full left-0 right-0 mb-2 bg-card border rounded-xl shadow-lg overflow-hidden z-50 animate-slide-up">
                                <button
                                    onClick={() => navigate('/')}
                                    className="w-full flex items-center gap-2 px-4 py-2.5 text-sm hover:bg-muted text-left transition-colors"
                                >
                                    <Home size={15} className="text-muted-foreground" /> Landing Page
                                </button>
                                <button
                                    onClick={() => navigate('/settings')}
                                    className="w-full flex items-center gap-2 px-4 py-2.5 text-sm hover:bg-muted text-left transition-colors"
                                >
                                    <Settings size={15} className="text-muted-foreground" /> Settings
                                </button>
                                <div className="border-t" />
                                <button
                                    onClick={handleLogout}
                                    className="w-full flex items-center gap-2 px-4 py-2.5 text-sm hover:bg-destructive/10 text-destructive text-left transition-colors"
                                >
                                    <LogOut size={15} /> Sign out
                                </button>
                            </div>
                        )}
                    </div>
                </div>
            </aside>

            {/* ── Main Area ── */}
            <div className="flex-1 flex flex-col overflow-hidden">

                {/* Top Header */}
                <header className="h-16 border-b bg-card/80 backdrop-blur-sm flex items-center justify-between px-6 flex-shrink-0 gap-4">
                    {/* Search */}
                    <div className="flex-1 max-w-sm hidden md:flex items-center gap-2 bg-muted rounded-xl px-3 py-1.5 focus-within:ring-1 focus-within:ring-primary transition-all">
                        <Search size={15} className="text-muted-foreground flex-shrink-0" />
                        <input
                            type="text"
                            placeholder="Search customers, invoices..."
                            value={searchQuery}
                            onChange={e => setSearchQuery(e.target.value)}
                            className="flex-1 bg-transparent text-sm focus:outline-none placeholder:text-muted-foreground"
                        />
                        {searchQuery && (
                            <button onClick={() => setSearchQuery('')} className="text-muted-foreground hover:text-foreground text-xs">✕</button>
                        )}
                    </div>

                    <div className="flex items-center gap-2 ml-auto">
                        {/* Notifications */}
                        <Button
                            variant="ghost"
                            size="icon"
                            className="rounded-xl relative"
                            onClick={() => navigate('/notifications')}
                        >
                            <Bell size={18} />
                            <span className="absolute top-1.5 right-1.5 w-2 h-2 bg-red-500 rounded-full" />
                        </Button>

                        {/* Theme toggle */}
                        <Button
                            variant="ghost"
                            size="icon"
                            onClick={() => setTheme(theme === 'light' ? 'dark' : 'light')}
                            className="rounded-xl"
                            title={`Switch to ${theme === 'light' ? 'dark' : 'light'} mode`}
                        >
                            {theme === 'light' ? <Moon size={18} /> : <Sun size={18} />}
                        </Button>

                        {/* Avatar */}
                        <div
                            className="h-9 w-9 rounded-xl bg-primary/20 flex items-center justify-center text-primary font-bold text-sm cursor-pointer hover:bg-primary/30 transition-colors"
                            onClick={() => navigate('/settings')}
                            title="Account settings"
                        >
                            {userInitial}
                        </div>
                    </div>
                </header>

                {/* Content */}
                <main className="flex-1 overflow-auto bg-muted/20">
                    <div className="p-6 animate-fade-in">
                        <Outlet />
                    </div>
                </main>
            </div>
        </div>
    );
};
