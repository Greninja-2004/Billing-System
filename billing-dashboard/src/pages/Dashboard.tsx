import { API_BASE_URL } from "../config";
import { useState, useEffect, useRef } from 'react';
import { Card, CardContent, CardHeader, CardTitle, CardDescription } from '../components/ui/card';
import { Button } from '../components/ui/button';
import { Badge } from '../components/ui/badge';
import {
    DollarSign, FileText, AlertCircle, Users, ArrowUpRight, ArrowDownRight,
    Plus, TrendingUp, Activity, Zap, CreditCard, RefreshCw, Bell,
    ChevronRight, MoreHorizontal
} from 'lucide-react';
import {
    LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip as RechartsTooltip,
    ResponsiveContainer, AreaChart, Area, PieChart, Pie, Cell, BarChart, Bar
} from 'recharts';

// --- DATA ---
const revenueData = [
    { name: 'Jan', value: 45000, prev: 38000 },
    { name: 'Feb', value: 52000, prev: 42000 },
    { name: 'Mar', value: 48000, prev: 47000 },
    { name: 'Apr', value: 61000, prev: 50000 },
    { name: 'May', value: 59000, prev: 55000 },
    { name: 'Jun', value: 68000, prev: 60000 },
    { name: 'Jul', value: 72000, prev: 65000 },
    { name: 'Aug', value: 85000, prev: 70000 },
    { name: 'Sep', value: 82000, prev: 78000 },
    { name: 'Oct', value: 94000, prev: 85000 },
    { name: 'Nov', value: 105000, prev: 92000 },
    { name: 'Dec', value: 125000, prev: 100000 },
];

const weeklyData = [
    { day: 'Mon', amount: 12400 },
    { day: 'Tue', amount: 8900 },
    { day: 'Wed', amount: 15200 },
    { day: 'Thu', amount: 11600 },
    { day: 'Fri', amount: 18900 },
    { day: 'Sat', amount: 6200 },
    { day: 'Sun', amount: 4800 },
];

const statusData = [
    { name: 'Paid', value: 65, color: '#10b981' },
    { name: 'Pending', value: 20, color: '#f59e0b' },
    { name: 'Overdue', value: 10, color: '#ef4444' },
    { name: 'Partial', value: 5, color: '#6366f1' },
];

interface Payment {
    id: string;
    customerName: string;
    amount: number;
    status: string;
    date: string;
    method?: string;
}

// Animated number counter hook
function useCountUp(end: number, duration = 1200) {
    const [count, setCount] = useState(0);
    const ref = useRef<ReturnType<typeof setTimeout> | null>(null);
    useEffect(() => {
        const start = Date.now();
        const step = () => {
            const elapsed = Date.now() - start;
            const progress = Math.min(elapsed / duration, 1);
            const eased = 1 - Math.pow(1 - progress, 3);
            setCount(Math.round(eased * end));
            if (progress < 1) ref.current = setTimeout(step, 16);
        };
        step();
        return () => { if (ref.current) clearTimeout(ref.current); };
    }, [end, duration]);
    return count;
}

// Status badge helper
function StatusBadge({ status }: { status: string }) {
    const map: Record<string, string> = {
        Completed: 'bg-emerald-500/10 text-emerald-600 border-emerald-500/20',
        Failed: 'bg-red-500/10 text-red-600 border-red-500/20',
        Pending: 'bg-amber-500/10 text-amber-600 border-amber-500/20',
        Refunded: 'bg-purple-500/10 text-purple-600 border-purple-500/20',
        Partial: 'bg-blue-500/10 text-blue-600 border-blue-500/20',
    };
    return (
        <span className={`inline-flex items-center px-2 py-0.5 rounded-full text-xs font-medium border ${map[status] ?? 'bg-muted text-muted-foreground border-border'}`}>
            {status}
        </span>
    );
}

// Mini sparkline bars
function SparkBars({ data, color = '#6366f1' }: { data: number[]; color?: string }) {
    const max = Math.max(...data);
    return (
        <div className="flex items-end gap-0.5 h-8">
            {data.map((v, i) => (
                <div
                    key={i}
                    className="w-1.5 rounded-sm transition-all duration-300"
                    style={{
                        height: `${(v / max) * 100}%`,
                        backgroundColor: color,
                        opacity: i === data.length - 1 ? 1 : 0.45 + (i / data.length) * 0.4,
                    }}
                />
            ))}
        </div>
    );
}

// KPI Card component
function KpiCard({
    title, value, prefix = '', suffix = '', change, changeLabel, icon: Icon,
    color = 'primary', sparkData, delay = 0
}: {
    title: string; value: number; prefix?: string; suffix?: string;
    change: number; changeLabel: string; icon: any;
    color?: string; sparkData: number[]; delay?: number;
}) {
    const count = useCountUp(value, 1200 + delay);
    const isPositive = change >= 0;

    const colorMap: Record<string, { bg: string; text: string; spark: string }> = {
        primary: { bg: 'bg-blue-500/10', text: 'text-blue-500', spark: '#3b82f6' },
        success: { bg: 'bg-emerald-500/10', text: 'text-emerald-500', spark: '#10b981' },
        destructive: { bg: 'bg-red-500/10', text: 'text-red-500', spark: '#ef4444' },
        purple: { bg: 'bg-purple-500/10', text: 'text-purple-500', spark: '#8b5cf6' },
    };
    const c = colorMap[color] ?? colorMap.primary;

    return (
        <div className="animate-slide-up" style={{ animationDelay: `${delay}ms` }}>
            <Card className="kpi-card group cursor-default">
                <CardHeader className="flex flex-row items-center justify-between space-y-0 pb-2">
                    <CardTitle className="text-sm font-medium text-muted-foreground">{title}</CardTitle>
                    <div className={`h-9 w-9 rounded-lg ${c.bg} flex items-center justify-center transition-transform duration-300 group-hover:scale-110`}>
                        <Icon className={`h-4 w-4 ${c.text}`} />
                    </div>
                </CardHeader>
                <CardContent>
                    <div className="flex items-end justify-between">
                        <div>
                            <div className="text-3xl font-bold tracking-tight">
                                {prefix}{typeof count === 'number' && count >= 1000 ? count.toLocaleString() : count}{suffix}
                            </div>
                            <p className="text-xs text-muted-foreground flex items-center mt-1.5 gap-1">
                                <span className={`flex items-center gap-0.5 font-medium ${isPositive ? 'text-emerald-500' : 'text-red-500'}`}>
                                    {isPositive ? <ArrowUpRight className="h-3 w-3" /> : <ArrowDownRight className="h-3 w-3" />}
                                    {Math.abs(change)}%
                                </span>
                                {changeLabel}
                            </p>
                        </div>
                        <SparkBars data={sparkData} color={c.spark} />
                    </div>
                </CardContent>
            </Card>
        </div>
    );
}

// Custom Recharts tooltip
const CustomTooltip = ({ active, payload, label }: any) => {
    if (active && payload && payload.length) {
        return (
            <div className="glass-card rounded-lg p-3 shadow-lg text-sm">
                <p className="font-semibold text-foreground mb-1">{label}</p>
                {payload.map((p: any, i: number) => (
                    <p key={i} style={{ color: p.color }} className="text-xs">
                        {p.name}: ${(p.value / 1000).toFixed(1)}k
                    </p>
                ))}
            </div>
        );
    }
    return null;
};

export const Dashboard = () => {
    const [recentTransactions, setRecentTransactions] = useState<Payment[]>([]);
    const [isLoading, setIsLoading] = useState(true);
    const [lastUpdated, setLastUpdated] = useState(new Date());
    const [isRefreshing, setIsRefreshing] = useState(false);

    const fetchData = () => {
        setIsRefreshing(true);
        fetch(`${API_BASE_URL}/api/payments`)
            .then(res => res.json())
            .then(data => {
                setRecentTransactions(data.slice(0, 6));
                setLastUpdated(new Date());
                setIsLoading(false);
                setIsRefreshing(false);
            })
            .catch(error => {
                console.error("Error fetching transactions:", error);
                setIsLoading(false);
                setIsRefreshing(false);
            });
    };

    useEffect(() => { fetchData(); }, []);

    // Quick action handlers
    const quickActions = [
        { label: 'New Invoice', icon: FileText, color: 'text-blue-500', bg: 'bg-blue-500/10', action: () => alert('Opening New Invoice wizard...') },
        { label: 'Add Customer', icon: Users, color: 'text-emerald-500', bg: 'bg-emerald-500/10', action: () => alert('Opening Add Customer form...') },
        { label: 'Record Payment', icon: CreditCard, color: 'text-purple-500', bg: 'bg-purple-500/10', action: () => alert('Record Payment modal opening...') },
        { label: 'Send Reminder', icon: Bell, color: 'text-amber-500', bg: 'bg-amber-500/10', action: () => alert('Sending payment reminders...') },
    ];

    return (
        <div className="space-y-6 pb-8">
            {/* ── Header ── */}
            <div className="flex flex-col sm:flex-row justify-between items-start sm:items-center gap-4 animate-slide-up">
                <div>
                    <h1 className="text-3xl font-bold tracking-tight">
                        Dashboard
                    </h1>
                    <p className="text-muted-foreground mt-1 text-sm flex items-center gap-2">
                        <span className="pulse-dot bg-emerald-500 text-emerald-500" />
                        Live · Last updated {lastUpdated.toLocaleTimeString()}
                    </p>
                </div>
                <div className="flex gap-2 flex-wrap">
                    <Button
                        variant="outline"
                        size="sm"
                        className="hover-lift gap-2"
                        onClick={() => window.print()}
                    >
                        <FileText className="h-4 w-4" /> Export
                    </Button>
                    <Button
                        variant="outline"
                        size="sm"
                        className="hover-lift gap-2"
                        onClick={fetchData}
                        disabled={isRefreshing}
                    >
                        <RefreshCw className={`h-4 w-4 ${isRefreshing ? 'animate-spin' : ''}`} />
                        Refresh
                    </Button>
                    <Button size="sm" className="hover-lift gap-2" onClick={() => alert('Opening New Invoice wizard...')}>
                        <Plus className="h-4 w-4" /> New Invoice
                    </Button>
                </div>
            </div>

            {/* ── KPI Cards ── */}
            <div className="grid gap-4 md:grid-cols-2 lg:grid-cols-4 stagger">
                <KpiCard
                    title="Total Revenue (YTD)"
                    value={1245600}
                    prefix="$"
                    change={24.5}
                    changeLabel="vs last year"
                    icon={DollarSign}
                    color="success"
                    sparkData={[45, 52, 48, 61, 59, 68, 72, 85, 82, 94, 105, 125]}
                    delay={0}
                />
                <KpiCard
                    title="Pending Invoices"
                    value={142}
                    change={-4.2}
                    changeLabel="from last month"
                    icon={FileText}
                    color="primary"
                    sparkData={[160, 155, 148, 152, 145, 142]}
                    delay={80}
                />
                <KpiCard
                    title="Overdue Accounts"
                    value={24}
                    change={12}
                    changeLabel="requires action"
                    icon={AlertCircle}
                    color="destructive"
                    sparkData={[18, 20, 22, 21, 23, 24]}
                    delay={160}
                />
                <KpiCard
                    title="Active Subscriptions"
                    value={1894}
                    change={8.3}
                    changeLabel="growth this month"
                    icon={Users}
                    color="purple"
                    sparkData={[1600, 1700, 1750, 1800, 1850, 1894]}
                    delay={240}
                />
            </div>

            {/* ── Quick Actions ── */}
            <div className="animate-slide-up" style={{ animationDelay: '300ms' }}>
                <div className="grid grid-cols-2 sm:grid-cols-4 gap-3">
                    {quickActions.map(({ label, icon: Icon, color, bg, action }) => (
                        <button
                            key={label}
                            onClick={action}
                            className={`flex flex-col items-center justify-center gap-2 p-4 rounded-xl border bg-card hover:${bg} hover:border-transparent transition-all duration-200 hover-lift group text-center`}
                        >
                            <div className={`h-10 w-10 rounded-lg ${bg} flex items-center justify-center transition-transform duration-200 group-hover:scale-110`}>
                                <Icon className={`h-5 w-5 ${color}`} />
                            </div>
                            <span className="text-sm font-medium">{label}</span>
                        </button>
                    ))}
                </div>
            </div>

            {/* ── Charts Row ── */}
            <div className="grid gap-4 md:grid-cols-2 lg:grid-cols-7 animate-slide-up" style={{ animationDelay: '320ms' }}>

                {/* Revenue Area Chart */}
                <Card className="lg:col-span-4 hover-lift transition-all duration-300">
                    <CardHeader className="flex flex-row items-start justify-between pb-2">
                        <div>
                            <CardTitle className="flex items-center gap-2">
                                <TrendingUp className="h-4 w-4 text-primary" />
                                Revenue Forecast
                            </CardTitle>
                            <CardDescription className="mt-1">Monthly recurring revenue vs prior year</CardDescription>
                        </div>
                        <Badge variant="outline" className="bg-emerald-500/10 text-emerald-600 border-emerald-500/20 text-xs">
                            ↑ 24.5% YoY
                        </Badge>
                    </CardHeader>
                    <CardContent className="pl-0 pt-0">
                        <div className="h-[280px] w-full">
                            <ResponsiveContainer width="100%" height="100%">
                                <AreaChart data={revenueData} margin={{ top: 10, right: 20, left: 10, bottom: 0 }}>
                                    <defs>
                                        <linearGradient id="rev" x1="0" y1="0" x2="0" y2="1">
                                            <stop offset="5%" stopColor="hsl(var(--primary))" stopOpacity={0.3} />
                                            <stop offset="95%" stopColor="hsl(var(--primary))" stopOpacity={0} />
                                        </linearGradient>
                                        <linearGradient id="prev" x1="0" y1="0" x2="0" y2="1">
                                            <stop offset="5%" stopColor="#6366f1" stopOpacity={0.15} />
                                            <stop offset="95%" stopColor="#6366f1" stopOpacity={0} />
                                        </linearGradient>
                                    </defs>
                                    <CartesianGrid strokeDasharray="3 3" vertical={false} stroke="hsl(var(--muted-foreground)/0.15)" />
                                    <XAxis dataKey="name" stroke="hsl(var(--muted-foreground))" fontSize={11} tickLine={false} axisLine={false} />
                                    <YAxis stroke="hsl(var(--muted-foreground))" fontSize={11} tickLine={false} axisLine={false} tickFormatter={v => `$${v / 1000}k`} />
                                    <RechartsTooltip content={<CustomTooltip />} />
                                    <Area type="monotone" dataKey="prev" name="Prior Year" stroke="#6366f1" strokeWidth={1.5} fill="url(#prev)" dot={false} strokeDasharray="4 2" />
                                    <Area type="monotone" dataKey="value" name="This Year" stroke="hsl(var(--primary))" strokeWidth={2.5} fill="url(#rev)" dot={false} activeDot={{ r: 5, strokeWidth: 0 }} />
                                </AreaChart>
                            </ResponsiveContainer>
                        </div>
                    </CardContent>
                </Card>

                {/* Invoice Status Donut */}
                <Card className="lg:col-span-3 hover-lift transition-all duration-300">
                    <CardHeader>
                        <CardTitle className="flex items-center gap-2">
                            <Activity className="h-4 w-4 text-primary" />
                            Invoice Breakdown
                        </CardTitle>
                        <CardDescription>Payment distribution across all invoices</CardDescription>
                    </CardHeader>
                    <CardContent>
                        <div className="h-[180px] w-full flex justify-center">
                            <ResponsiveContainer width="100%" height="100%">
                                <PieChart>
                                    <Pie
                                        data={statusData}
                                        innerRadius={55}
                                        outerRadius={78}
                                        paddingAngle={4}
                                        dataKey="value"
                                        stroke="none"
                                        startAngle={90}
                                        endAngle={450}
                                    >
                                        {statusData.map((entry, index) => (
                                            <Cell key={`cell-${index}`} fill={entry.color} />
                                        ))}
                                    </Pie>
                                    <RechartsTooltip formatter={(val) => [`${val}%`, '']} />
                                </PieChart>
                            </ResponsiveContainer>
                        </div>

                        <div className="grid grid-cols-2 gap-3 mt-2">
                            {statusData.map((item) => (
                                <div key={item.name} className="flex items-center gap-2 p-2 rounded-lg hover:bg-muted/50 transition-colors cursor-default">
                                    <div className="w-2.5 h-2.5 rounded-full flex-shrink-0" style={{ backgroundColor: item.color }} />
                                    <div className="flex-1 min-w-0">
                                        <p className="text-xs font-medium truncate">{item.name}</p>
                                    </div>
                                    <span className="text-xs font-bold text-muted-foreground">{item.value}%</span>
                                </div>
                            ))}
                        </div>
                    </CardContent>
                </Card>
            </div>

            {/* ── Weekly Bar Chart + Transactions ── */}
            <div className="grid gap-4 lg:grid-cols-3 animate-slide-up" style={{ animationDelay: '380ms' }}>

                {/* Weekly volume */}
                <Card className="hover-lift">
                    <CardHeader>
                        <CardTitle className="flex items-center gap-2 text-base">
                            <Zap className="h-4 w-4 text-amber-500" />
                            This Week
                        </CardTitle>
                        <CardDescription>Daily transaction volume</CardDescription>
                    </CardHeader>
                    <CardContent className="pl-0">
                        <div className="h-[180px]">
                            <ResponsiveContainer width="100%" height="100%">
                                <BarChart data={weeklyData} margin={{ top: 0, right: 10, left: 0, bottom: 0 }} barCategoryGap="30%">
                                    <CartesianGrid strokeDasharray="3 3" vertical={false} stroke="hsl(var(--muted-foreground)/0.12)" />
                                    <XAxis dataKey="day" stroke="hsl(var(--muted-foreground))" fontSize={11} tickLine={false} axisLine={false} />
                                    <YAxis stroke="hsl(var(--muted-foreground))" fontSize={11} tickLine={false} axisLine={false} tickFormatter={v => `$${v / 1000}k`} />
                                    <RechartsTooltip contentStyle={{ backgroundColor: 'hsl(var(--card))', borderColor: 'hsl(var(--border))', borderRadius: '8px', fontSize: '12px' }} />
                                    <Bar dataKey="amount" name="Revenue" fill="hsl(var(--primary))" radius={[4, 4, 0, 0]} />
                                </BarChart>
                            </ResponsiveContainer>
                        </div>
                        <div className="flex items-center justify-between mt-3 px-2">
                            <span className="text-xs text-muted-foreground">Total</span>
                            <span className="text-sm font-bold">${weeklyData.reduce((a, b) => a + b.amount, 0).toLocaleString()}</span>
                        </div>
                    </CardContent>
                </Card>

                {/* Recent Payment Activity */}
                <Card className="lg:col-span-2 hover-lift transition-all duration-300">
                    <CardHeader className="flex flex-row items-center justify-between">
                        <div>
                            <CardTitle className="flex items-center gap-2">
                                <Activity className="h-4 w-4 text-primary" />
                                Recent Transactions
                            </CardTitle>
                            <CardDescription>Live feed of latest payments</CardDescription>
                        </div>
                        <Button variant="ghost" size="sm" className="gap-1 text-xs">
                            View all <ChevronRight className="h-3 w-3" />
                        </Button>
                    </CardHeader>
                    <CardContent>
                        <div className="space-y-1">
                            {isLoading ? (
                                // Shimmer skeleton
                                Array.from({ length: 5 }).map((_, i) => (
                                    <div key={i} className="flex items-center justify-between p-3 rounded-lg gap-4">
                                        <div className="flex items-center gap-3 flex-1">
                                            <div className="shimmer w-8 h-8 rounded-full flex-shrink-0" />
                                            <div className="space-y-1.5 flex-1">
                                                <div className="shimmer h-3 w-28 rounded" />
                                                <div className="shimmer h-2.5 w-20 rounded" />
                                            </div>
                                        </div>
                                        <div className="shimmer h-5 w-16 rounded-full" />
                                        <div className="shimmer h-4 w-16 rounded" />
                                    </div>
                                ))
                            ) : recentTransactions.length === 0 ? (
                                <div className="text-center py-8 text-muted-foreground text-sm">No transactions found</div>
                            ) : (
                                recentTransactions.map((tx, i) => (
                                    <div
                                        key={tx.id}
                                        className="table-row-hover flex items-center justify-between p-3 rounded-lg border border-transparent hover:border-border/50 group animate-slide-up"
                                        style={{ animationDelay: `${i * 60}ms` }}
                                    >
                                        <div className="flex items-center gap-3 flex-1 min-w-0">
                                            <div className={`w-9 h-9 rounded-full flex items-center justify-center flex-shrink-0 text-xs font-bold ${tx.status === 'Completed' ? 'bg-emerald-500/10 text-emerald-600' :
                                                    tx.status === 'Failed' ? 'bg-red-500/10 text-red-600' :
                                                        'bg-amber-500/10 text-amber-600'
                                                }`}>
                                                {(tx.customerName || '?').substring(0, 2).toUpperCase()}
                                            </div>
                                            <div className="min-w-0">
                                                <p className="text-sm font-medium truncate">{tx.customerName}</p>
                                                <p className="text-xs text-muted-foreground truncate">{new Date(tx.date).toLocaleDateString('en-US', { month: 'short', day: 'numeric', year: 'numeric' })}</p>
                                            </div>
                                        </div>

                                        <div className="flex items-center gap-3 flex-shrink-0">
                                            <StatusBadge status={tx.status} />
                                            <div className="text-right">
                                                <p className="text-sm font-bold">${tx.amount.toFixed(2)}</p>
                                            </div>
                                            <Button variant="ghost" size="icon" className="h-7 w-7 opacity-0 group-hover:opacity-100 transition-opacity">
                                                <MoreHorizontal className="h-4 w-4" />
                                            </Button>
                                        </div>
                                    </div>
                                ))
                            )}
                        </div>
                    </CardContent>
                </Card>
            </div>

            {/* ── Goal Progress ── */}
            <Card className="hover-lift animate-slide-up" style={{ animationDelay: '420ms' }}>
                <CardHeader>
                    <CardTitle className="flex items-center gap-2">
                        <TrendingUp className="h-4 w-4 text-primary" />
                        Annual Goals
                    </CardTitle>
                    <CardDescription>Progress towards financial targets for FY 2026</CardDescription>
                </CardHeader>
                <CardContent>
                    <div className="grid sm:grid-cols-3 gap-6">
                        {[
                            { label: 'Revenue Goal', current: 1245600, target: 1500000, color: 'bg-blue-500' },
                            { label: 'New Customers', current: 856, target: 1000, color: 'bg-emerald-500' },
                            { label: 'Avg Invoice Value', current: 2840, target: 3500, color: 'bg-purple-500' },
                        ].map(({ label, current, target, color }) => {
                            const pct = Math.min(Math.round((current / target) * 100), 100);
                            return (
                                <div key={label} className="space-y-2">
                                    <div className="flex justify-between text-sm">
                                        <span className="font-medium">{label}</span>
                                        <span className="font-bold text-muted-foreground">{pct}%</span>
                                    </div>
                                    <div className="h-2 bg-muted rounded-full overflow-hidden">
                                        <div
                                            className={`h-full ${color} rounded-full progress-bar-fill`}
                                            style={{ '--target-width': `${pct}%` } as React.CSSProperties}
                                        />
                                    </div>
                                    <p className="text-xs text-muted-foreground">
                                        {current >= 1000 ? `$${(current / 1000).toFixed(1)}k` : current} of {target >= 1000 ? `$${(target / 1000).toFixed(0)}k` : target}
                                    </p>
                                </div>
                            );
                        })}
                    </div>
                </CardContent>
            </Card>
        </div>
    );
};
