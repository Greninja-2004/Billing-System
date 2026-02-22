import { Bell, AlertTriangle, Info, CheckCircle2 } from 'lucide-react';
import { Card, CardHeader, CardTitle, CardDescription, CardContent } from '../components/ui/card';
import { Badge } from '../components/ui/badge';
import { Button } from '../components/ui/button';

const MOCK_NOTIFICATIONS = [
    { id: 1, type: 'CRITICAL', message: 'Fraud threshold exceeded for Customer CUST-002', time: '10 mins ago' },
    { id: 2, type: 'HIGH', message: 'Invoice INV-2024-1005 is now 30+ days overdue', time: '1 hour ago' },
    { id: 3, type: 'MEDIUM', message: 'Payment gateway CreditCard response latency high', time: '3 hours ago' },
    { id: 4, type: 'LOW', message: 'Batch invoice generation completed successfully', time: '5 hours ago' },
    { id: 5, type: 'INFO', message: 'System audit log backup complete', time: '1 day ago' },
];

export const Notifications = () => {
    const getSeverityIcon = (type: string) => {
        switch (type) {
            case 'CRITICAL': return <AlertTriangle className="h-5 w-5 text-destructive" />;
            case 'HIGH': return <AlertTriangle className="h-5 w-5 text-amber-500" />;
            case 'MEDIUM': return <AlertTriangle className="h-5 w-5 text-blue-500" />;
            case 'LOW':
            case 'INFO': return <Info className="h-5 w-5 text-muted-foreground" />;
            case 'SUCCESS': return <CheckCircle2 className="h-5 w-5 text-emerald-500" />;
            default: return <Bell className="h-5 w-5 text-muted-foreground" />;
        }
    };

    const getSeverityBadge = (type: string) => {
        switch (type) {
            case 'CRITICAL': return <Badge variant="destructive">CRITICAL</Badge>;
            case 'HIGH': return <Badge className="bg-amber-500 hover:bg-amber-600">HIGH</Badge>;
            case 'MEDIUM': return <Badge className="bg-blue-500 hover:bg-blue-600">MEDIUM</Badge>;
            case 'LOW': return <Badge variant="secondary">LOW</Badge>;
            default: return <Badge variant="outline">{type}</Badge>;
        }
    };

    return (
        <div className="space-y-6 max-w-4xl mx-auto">
            <div className="flex flex-col sm:flex-row justify-between items-start sm:items-center space-y-4 sm:space-y-0">
                <div>
                    <h2 className="text-3xl font-bold tracking-tight">Notifications Center</h2>
                    <p className="text-muted-foreground mt-1">System alerts, process escalations, and infrastructure warnings.</p>
                </div>
                <Button variant="outline">Mark All as Read</Button>
            </div>

            <Card>
                <CardHeader className="pb-3 border-b">
                    <CardTitle>Recent Alerts</CardTitle>
                    <CardDescription>Escalations from the C++ Billing State Machine</CardDescription>
                </CardHeader>
                <CardContent className="p-0">
                    <div className="divide-y divide-border">
                        {MOCK_NOTIFICATIONS.map(alert => (
                            <div key={alert.id} className="p-4 flex flex-col sm:flex-row items-start sm:items-center justify-between hover:bg-muted/50 transition-colors">
                                <div className="flex items-start space-x-4 mb-2 sm:mb-0">
                                    <div className="mt-1">{getSeverityIcon(alert.type)}</div>
                                    <div className="space-y-1">
                                        <p className="text-sm font-medium leading-none">{alert.message}</p>
                                        <p className="text-xs text-muted-foreground">{alert.time}</p>
                                    </div>
                                </div>
                                <div className="flex items-center space-x-4 mt-2 sm:mt-0 pl-9 sm:pl-0">
                                    {getSeverityBadge(alert.type)}
                                    <Button variant="ghost" size="sm" className="h-8">Dismiss</Button>
                                </div>
                            </div>
                        ))}
                    </div>
                </CardContent>
            </Card>
        </div>
    );
};
