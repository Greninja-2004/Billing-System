import { Card, CardContent, CardHeader, CardTitle, CardDescription } from '../components/ui/card';
import { Button } from '../components/ui/button';
import { Download, Calendar, TrendingUp } from 'lucide-react';
import {
    BarChart, Bar, XAxis, YAxis, CartesianGrid, Tooltip as RechartsTooltip, ResponsiveContainer,
    ComposedChart, Line
} from 'recharts';

const AGING_DATA = [
    { name: '0-30 days', value: 45000, fill: '#10b981' }, // emerald
    { name: '31-60 days', value: 12500, fill: '#f59e0b' }, // amber
    { name: '61-90 days', value: 4200, fill: '#f97316' }, // orange
    { name: '90+ days', value: 2800, fill: '#ef4444' }, // red
];

const FORECAST_DATA = [
    { month: 'Oct', actual: 94000, forecast: 90000 },
    { month: 'Nov', actual: 105000, forecast: 100000 },
    { month: 'Dec', actual: 125000, forecast: 115000 },
    { month: 'Jan', actual: 130000, forecast: 128000 },
    { month: 'Feb', actual: null, forecast: 135000 },
    { month: 'Mar', actual: null, forecast: 142000 },
];

const TOP_CUSTOMERS = [
    { rank: 1, name: 'Stark Industries', clv: '$125,000' },
    { rank: 2, name: 'Wayne Enterprises', clv: '$89,000' },
    { rank: 3, name: 'Acme Corporation', clv: '$45,000' },
    { rank: 4, name: 'Soylent Corp', clv: '$34,000' },
    { rank: 5, name: 'Dunder Mifflin', clv: '$12,400' },
];

export const Reports = () => {
    return (
        <div className="space-y-6">
            <div className="flex flex-col sm:flex-row justify-between items-start sm:items-center space-y-4 sm:space-y-0">
                <div>
                    <h2 className="text-3xl font-bold tracking-tight">Analytics & Reports</h2>
                    <p className="text-muted-foreground mt-1">Aging analysis, revenue forecasting, and customer lifetime value.</p>
                </div>
                <div className="flex space-x-2">
                    <Button variant="outline"><Calendar className="mr-2 h-4 w-4" /> This Quarter</Button>
                    <Button><Download className="mr-2 h-4 w-4" /> Export PDF</Button>
                </div>
            </div>

            <div className="grid gap-6 md:grid-cols-2">
                {/* AR Aging Report */}
                <Card>
                    <CardHeader>
                        <CardTitle>Accounts Receivable Aging</CardTitle>
                        <CardDescription>Outstanding invoice amounts bucketed by days overdue.</CardDescription>
                    </CardHeader>
                    <CardContent>
                        <div className="h-[300px] w-full">
                            <ResponsiveContainer width="100%" height="100%">
                                <BarChart data={AGING_DATA} margin={{ top: 20, right: 30, left: 20, bottom: 5 }}>
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
                                        cursor={{ fill: 'transparent' }}
                                        contentStyle={{ backgroundColor: 'hsl(var(--card))', borderColor: 'hsl(var(--border))', borderRadius: '8px' }}
                                        itemStyle={{ color: 'hsl(var(--foreground))', fontWeight: 'bold' }}
                                        formatter={(value: any) => [`$${value.toLocaleString()}`, 'Amount']}
                                    />
                                    <Bar dataKey="value" radius={[4, 4, 0, 0]} />
                                </BarChart>
                            </ResponsiveContainer>
                        </div>
                        <div className="mt-4 text-center text-sm text-muted-foreground">
                            Total Outstanding: <span className="font-bold text-foreground">$64,500.00</span>
                        </div>
                    </CardContent>
                </Card>

                {/* Revenue Forecast */}
                <Card>
                    <CardHeader>
                        <CardTitle>Revenue Forecast (SMA)</CardTitle>
                        <CardDescription>3-month Simple Moving Average projection vs Actuals.</CardDescription>
                    </CardHeader>
                    <CardContent>
                        <div className="h-[300px] w-full">
                            <ResponsiveContainer width="100%" height="100%">
                                <ComposedChart data={FORECAST_DATA} margin={{ top: 20, right: 30, left: 20, bottom: 5 }}>
                                    <CartesianGrid strokeDasharray="3 3" vertical={false} stroke="hsl(var(--muted-foreground)/0.2)" />
                                    <XAxis dataKey="month" stroke="hsl(var(--muted-foreground))" fontSize={12} tickLine={false} axisLine={false} />
                                    <YAxis
                                        stroke="hsl(var(--muted-foreground))"
                                        fontSize={12}
                                        tickLine={false}
                                        axisLine={false}
                                        tickFormatter={(value) => `$${value / 1000}k`}
                                    />
                                    <RechartsTooltip
                                        contentStyle={{ backgroundColor: 'hsl(var(--card))', borderColor: 'hsl(var(--border))', borderRadius: '8px' }}
                                    />
                                    <Bar dataKey="actual" fill="hsl(var(--primary))" radius={[4, 4, 0, 0]} name="Actual Revenue" barSize={30} />
                                    <Line type="monotone" dataKey="forecast" stroke="hsl(var(--destructive))" strokeWidth={3} strokeDasharray="5 5" name="SMA Forecast" dot={{ r: 4 }} />
                                </ComposedChart>
                            </ResponsiveContainer>
                        </div>
                    </CardContent>
                </Card>
            </div>

            {/* CLV Leaderboard */}
            <Card>
                <CardHeader>
                    <div className="flex items-center justify-between">
                        <div>
                            <CardTitle>Top Customers by Lifetime Value</CardTitle>
                            <CardDescription>Highest revenue-generating enterprise accounts.</CardDescription>
                        </div>
                        <TrendingUp className="text-emerald-500 h-6 w-6" />
                    </div>
                </CardHeader>
                <CardContent>
                    <div className="space-y-4">
                        {TOP_CUSTOMERS.map((cust) => (
                            <div key={cust.rank} className="flex items-center justify-between p-3 rounded-lg bg-muted/30">
                                <div className="flex items-center space-x-4">
                                    <div className="w-8 h-8 rounded-full bg-primary/20 flex items-center justify-center text-primary font-bold">
                                        #{cust.rank}
                                    </div>
                                    <div>
                                        <p className="text-sm font-semibold">{cust.name}</p>
                                        <p className="text-xs text-muted-foreground">Enterprise Tier</p>
                                    </div>
                                </div>
                                <div className="font-bold text-emerald-600 dark:text-emerald-400">
                                    {cust.clv}
                                </div>
                            </div>
                        ))}
                    </div>
                </CardContent>
            </Card>
        </div>
    );
};
