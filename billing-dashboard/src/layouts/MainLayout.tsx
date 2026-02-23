import { Outlet, NavLink, useNavigate } from 'react-router-dom';
import { useAuthStore } from '../store/auth';
import {
    LayoutDashboard,
    Users,
    FileText,
    CreditCard,
    BarChart3,
    Bell,
    LogOut,
    Settings,
    ShieldAlert,
    Moon,
    Sun
} from 'lucide-react';
import { Button } from '../components/ui/button';
import { useEffect, useState } from 'react';

export const MainLayout = () => {
    const { user, logout } = useAuthStore();
    const navigate = useNavigate();
    const [theme, setTheme] = useState<'light' | 'dark'>(
        (localStorage.getItem('theme') as 'light' | 'dark') || 'light'
    );

    useEffect(() => {
        const root = window.document.documentElement;
        root.classList.remove('light', 'dark');
        root.classList.add(theme);
        localStorage.setItem('theme', theme);
    }, [theme]);

    const toggleTheme = () => {
        setTheme(theme === 'light' ? 'dark' : 'light');
    };

    const handleLogout = () => {
        logout();
        navigate('/login');
    };

    const navItems = [
        { name: 'Dashboard', path: '/dashboard', icon: <LayoutDashboard size={20} />, roles: ['ADMIN', 'MANAGER', 'VIEWER'] },
        { name: 'Customers', path: '/customers', icon: <Users size={20} />, roles: ['ADMIN', 'MANAGER', 'VIEWER'] },
        { name: 'Invoices', path: '/invoices', icon: <FileText size={20} />, roles: ['ADMIN', 'MANAGER', 'VIEWER'] },
        { name: 'Payments', path: '/payments', icon: <CreditCard size={20} />, roles: ['ADMIN', 'MANAGER', 'VIEWER'] },
        { name: 'Reports', path: '/reports', icon: <BarChart3 size={20} />, roles: ['ADMIN', 'MANAGER', 'VIEWER'] },
        { name: 'Notifications', path: '/notifications', icon: <Bell size={20} />, roles: ['ADMIN', 'MANAGER', 'VIEWER'] },
        { name: 'Users', path: '/users', icon: <ShieldAlert size={20} />, roles: ['ADMIN'] },
        { name: 'Audit Log', path: '/audit', icon: <FileText size={20} />, roles: ['ADMIN', 'MANAGER'] },
        { name: 'Settings', path: '/settings', icon: <Settings size={20} />, roles: ['ADMIN', 'MANAGER', 'VIEWER'] },
    ];

    const filteredNav = navItems.filter((item) => user && item.roles.includes(user.role));

    return (
        <div className="flex h-screen overflow-hidden bg-background">
            {/* Sidebar */}
            <aside className="w-64 border-r bg-card flex-shrink-0 flex flex-col transition-all duration-300">
                <div className="h-16 flex items-center px-6 border-b">
                    <div className="h-8 w-8 rounded bg-primary flex items-center justify-center text-primary-foreground font-bold mr-3 shadow-md">
                        BS
                    </div>
                    <span className="text-lg font-bold tracking-tight">Billing Pro</span>
                </div>

                <div className="flex-1 overflow-y-auto py-4">
                    <nav className="space-y-1 px-3">
                        {filteredNav.map((item) => (
                            <NavLink
                                key={item.path}
                                to={item.path}
                                className={({ isActive }) =>
                                    `flex items-center px-3 py-2.5 text-sm font-medium rounded-md transition-colors ${isActive
                                        ? 'bg-primary/10 text-primary'
                                        : 'text-muted-foreground hover:bg-muted hover:text-foreground'
                                    }`
                                }
                            >
                                <div className="mr-3">{item.icon}</div>
                                {item.name}
                            </NavLink>
                        ))}
                    </nav>
                </div>

                <div className="p-4 border-t flex items-center justify-between">
                    <div className="flex flex-col truncate">
                        <span className="text-sm font-medium truncate">{user?.username}</span>
                        <span className="text-xs text-muted-foreground truncate">{user?.role}</span>
                    </div>
                    <Button variant="ghost" size="icon" onClick={handleLogout} title="Logout">
                        <LogOut size={18} />
                    </Button>
                </div>
            </aside>

            {/* Main Container */}
            <div className="flex-1 flex flex-col overflow-hidden">
                {/* Top Header */}
                <header className="h-16 border-b bg-card flex items-center justify-between px-6 flex-shrink-0">
                    <div className="flex-1 max-w-md hidden md:flex">
                        <input
                            type="text"
                            placeholder="Search customers, invoices..."
                            className="w-full bg-muted rounded-md px-4 py-2 text-sm focus:outline-none focus:ring-1 focus:ring-primary"
                        />
                    </div>
                    <div className="flex items-center space-x-4 ml-auto">
                        <Button variant="ghost" size="icon" onClick={toggleTheme} className="rounded-full">
                            {theme === 'light' ? <Moon size={20} /> : <Sun size={20} />}
                        </Button>
                        <div className="w-8 h-8 rounded-full bg-primary/20 flex items-center justify-center text-primary font-bold">
                            {user?.username.charAt(0).toUpperCase()}
                        </div>
                    </div>
                </header>

                {/* Content Area */}
                <main className="flex-1 overflow-auto bg-muted/20 p-6">
                    <Outlet />
                </main>
            </div>
        </div>
    );
};
