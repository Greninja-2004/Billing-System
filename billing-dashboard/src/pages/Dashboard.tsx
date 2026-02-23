import { API_BASE_URL } from "../config";
import { useState, useEffect } from 'react';
import { Card, CardContent, CardHeader, CardTitle, CardDescription } from '../components/ui/card';
import { Button } from '../components/ui/button';
import { Badge } from '../components/ui/badge';
import { DollarSign, FileText, AlertCircle, Users, ArrowUpRight, ArrowDownRight, Plus } from 'lucide-react';
import {
    LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip as RechartsTooltip, ResponsiveContainer,
    PieChart, Pie, Cell
} from 'recharts';

// --- MOCK DATA ---
const revenueData = [
    { name: 'Jan', value: 45000 },
    { name: 'Feb', value: 52000 },
    { name: 'Mar', value: 48000 },
    { name: 'Apr', value: 61000 },
    { name: 'May', value: 59000 },
    { name: 'Jun', value: 68000 },
    { name: 'Jul', value: 72000 },
    { name: 'Aug', value: 85000 },
    { name: 'Sep', value: 82000 },
    { name: 'Oct', value: 94000 },
    { name: 'Nov', value: 105000 },
    { name: 'Dec', value: 125000 },
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
}

export const Dashboard = () => {
    const [recentTransactions, setRecentTransactions] = useState<Payment[]>([]);

    useEffect(() => {
        fetch(`${API_BASE_URL}/api/payments`)
            .then(res => res.json())
            .then(data => {
                // Slice to just the 5 most recent
                setRecentTransactions(data.slice(0, 5));
            })
            .catch(error => {
                console.error("Error fetching transactions:", error);
            });
    }, []);

    return (
        <div className="space-y-6">
            {/* Header section with quick actions */}
            <div className="flex flex-col sm:flex-row justify-between items-start sm:items-center space-y-4 sm:space-y-0">
                <div>
                    <h2 className="text-3xl font-bold tracking-tight">Dashboard Overview</h2>
                    <p className="text-muted-foreground mt-1">Here's what's happening today in your billing ecosystem.</p>
                </div>
                <div className="flex space-x-2">
                    <Button variant="outline" className="hover-lift" onClick={() => window.print()}><FileText className="mr-2 h-4 w-4" /> Download Report</Button>
                    <Button className="hover-lift" onClick={() => alert('Opening New Invoice Modal...')}><Plus className="mr-2 h-4 w-4" /> New Invoice</Button>
                </div>
            </div>

            {/* KPI Cards */}
            <div className="grid gap-4 md:grid-cols-2 lg:grid-cols-4">
                {/* Total Revenue */}
                <Card className="hover-lift transition-all duration-300">
                    <CardHeader className="flex flex-row items-center justify-between space-y-0 pb-2">
                        <CardTitle className="text-sm font-medium">Total Revenue (YTD)</CardTitle>
                        <DollarSign className="h-4 w-4 text-muted-foreground" />
                    </CardHeader>
                    <CardContent>
                        <div className="text-2xl font-bold">$1,245,600.00</div>
                        <p className="text-xs text-muted-foreground flex items-center mt-1">
                            <span className="text-emerald-500 flex items-center mr-1">
                                <ArrowUpRight className="h-3 w-3 mr-1" />+24.5%
                            </span>
                            from last year
                        </p>
                    </CardContent>
                </Card>

                {/* Pending Invoices */}
                <Card className="hover-lift transition-all duration-300">
                    <CardHeader className="flex flex-row items-center justify-between space-y-0 pb-2">
                        <CardTitle className="text-sm font-medium">Pending Invoices</CardTitle>
                        <FileText className="h-4 w-4 text-muted-foreground" />
                    </CardHeader>
                    <CardContent>
                        <div className="text-2xl font-bold">142</div>
                        <p className="text-xs text-muted-foreground flex items-center mt-1">
                            <span className="text-emerald-500 flex items-center mr-1">
                                <ArrowDownRight className="h-3 w-3 mr-1" />-4.2%
                            </span>
                            from last month
                        </p>
                    </CardContent>
                </Card>

                {/* Overdue Accounts */}
                <Card className="hover-lift transition-all duration-300">
                    <CardHeader className="flex flex-row items-center justify-between space-y-0 pb-2">
                        <CardTitle className="text-sm font-medium">Overdue Accounts</CardTitle>
                        <AlertCircle className="h-4 w-4 text-destructive" />
                    </CardHeader>
                    <CardContent>
                        <div className="text-2xl font-bold text-destructive">24</div>
                        <p className="text-xs text-muted-foreground flex items-center mt-1">
                            <span className="text-destructive flex items-center mr-1">
                                <ArrowUpRight className="h-3 w-3 mr-1" />+12%
                            </span>
                            requires immediate action
                        </p>
                    </CardContent>
                </Card>

                {/* Active Customers */}
                <Card className="hover-lift transition-all duration-300">
                    <CardHeader className="flex flex-row items-center justify-between space-y-0 pb-2">
                        <CardTitle className="text-sm font-medium">Active Subscriptions</CardTitle>
                        <Users className="h-4 w-4 text-muted-foreground" />
                    </CardHeader>
                    <CardContent>
                        <div className="text-2xl font-bold">1,894</div>
                        <p className="text-xs text-muted-foreground flex items-center mt-1">
                            <span className="text-emerald-500 flex items-center mr-1">
                                <ArrowUpRight className="h-3 w-3 mr-1" />+8 new
                            </span>
                            this week
                        </p>
                    </CardContent>
                </Card>
            </div>

            <div className="grid gap-4 md:grid-cols-2 lg:grid-cols-7">

                {/* Main Revenue Chart */}
                <Card className="lg:col-span-4 hover-lift transition-all duration-300">
                    <CardHeader>
                        <CardTitle>Revenue Forecast & History</CardTitle>
                        <CardDescription>Visualizing MRR over the last 12 months with moving averages.</CardDescription>
                    </CardHeader>
                    <CardContent className="pl-0">
                        <div className="h-[300px] w-full">
                            <ResponsiveContainer width="100%" height="100%">
                                <LineChart data={revenueData} margin={{ top: 5, right: 30, left: 20, bottom: 5 }}>
                                    <CartesianGrid strokeDasharray="3 3" vertical={false} stroke="hsl(var(--muted-foreground)/0.2)" />
                                    <XAxis dataKey="name" stroke="hsl(var(--muted-foreground))" fontSize={12} tickLine={false} axisLine={false} />
                                    <YAxis
                                        stroke="hsl(var(--muted-foreground))"
                                        fontSize={12}
                                        tickLine={false}
                                        axisLine={false}
                                        tickFormatter={(value) => `$${value / 1000}k`}
                                    />
                                    <RechartsTooltip
                                        contentStyle={{ backgroundColor: 'hsl(var(--card))', borderColor: 'hsl(var(--border))', borderRadius: '8px' }}
                                        itemStyle={{ color: 'hsl(var(--foreground))' }}
                                    />
                                    <Line type="monotone" dataKey="value" stroke="hsl(var(--primary))" strokeWidth={3} dot={false} activeDot={{ r: 6 }} />
                                </LineChart>
                            </ResponsiveContainer>
                        </div>
                    </CardContent>
                </Card>

                {/* Invoice Status Breakdown */}
                <Card className="lg:col-span-3 hover-lift transition-all duration-300">
                    <CardHeader>
                        <CardTitle>Invoice Status Breakdown</CardTitle>
                        <CardDescription>Payment distribution across all outstanding invoices.</CardDescription>
                    </CardHeader>
                    <CardContent>
                        <div className="h-[200px] w-full flex justify-center">
                            <ResponsiveContainer width="100%" height="100%">
                                <PieChart>
                                    <Pie
                                        data={statusData}
                                        innerRadius={60}
                                        outerRadius={80}
                                        paddingAngle={5}
                                        dataKey="value"
                                        stroke="none"
                                    >
                                        {statusData.map((entry, index) => (
                                            <Cell key={`cell-${index}`} fill={entry.color} />
                                        ))}
                                    </Pie>
                                    <RechartsTooltip />
                                </PieChart>
                            </ResponsiveContainer>
                        </div>

                        {/* Custom Legend */}
                        <div className="grid grid-cols-2 gap-4 mt-6">
                            {statusData.map((item) => (
                                <div key={item.name} className="flex items-center">
                                    <div className="w-3 h-3 rounded-full mr-2" style={{ backgroundColor: item.color }} />
                                    <div className="flex-1 text-sm font-medium">{item.name}</div>
                                    <div className="text-sm text-muted-foreground">{item.value}%</div>
                                </div>
                            ))}
                        </div>
                    </CardContent>
                </Card>
            </div>

            {/* Recent Activity Feed */}
            <Card className="hover-lift transition-all duration-300">
                <CardHeader>
                    <CardTitle>Recent Payment Activity</CardTitle>
                    <CardDescription>Live feed of the latest transactions processed by the Billing Engine.</CardDescription>
                </CardHeader>
                <CardContent>
                    <div className="space-y-4">
                        {recentTransactions.length === 0 ? (
                            <div className="p-4 text-center text-muted-foreground">Loading recent activity...</div>
                        ) : (
                            recentTransactions.map((tx) => (
                                <div key={tx.id} className="flex items-center justify-between p-4 rounded-lg border bg-card hover:bg-muted/50 transition-colors">
                                    <div className="flex items-center space-x-4">
                                        <div className={`w-2 h-2 rounded-full ${tx.status === 'Completed' ? 'bg-emerald-500' : tx.status === 'Failed' ? 'bg-destructive' : 'bg-muted-foreground'}`} />
                                        <div>
                                            <p className="text-sm font-medium leading-none">{tx.customerName}</p>
                                            <p className="text-sm text-muted-foreground mt-1">{tx.id}</p>
                                        </div>
                                    </div>

                                    <div className="flex items-center space-x-4">
                                        <Badge variant={
                                            tx.status === 'Completed' ? 'success' :
                                                tx.status === 'Failed' ? 'destructive' : 'secondary'
                                        }>
                                            {tx.status}
                                        </Badge>
                                        <div className="text-right min-w-[100px]">
                                            <p className="text-sm font-bold">${tx.amount.toFixed(2)}</p>
                                            <p className="text-xs text-muted-foreground mt-1">{new Date(tx.date).toLocaleDateString()}</p>
                                        </div>
                                    </div>
                                </div>
                            ))
                        )}
                    </div>
                </CardContent>
            </Card>

        </div>
    );
};
