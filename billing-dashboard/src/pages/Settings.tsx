import { Card, CardHeader, CardTitle, CardDescription, CardContent, CardFooter } from '../components/ui/card';
import { Button } from '../components/ui/button';
import { Input } from '../components/ui/input';
import { Label } from '../components/ui/label';
import { Building2, Save } from 'lucide-react';

export const Settings = () => {
    const handleSubmit = (e: React.FormEvent) => {
        e.preventDefault();
        // mock save action
    };

    return (
        <div className="space-y-6 max-w-4xl mx-auto">
            <div>
                <h2 className="text-3xl font-bold tracking-tight">System Configuration</h2>
                <p className="text-muted-foreground mt-1">Manage billing rules, default taxes, and interface preferences.</p>
            </div>

            <div className="grid gap-6">
                <form onSubmit={handleSubmit}>
                    <Card>
                        <CardHeader>
                            <div className="flex items-center space-x-2">
                                <Building2 className="h-5 w-5 text-muted-foreground" />
                                <CardTitle>Company Profile</CardTitle>
                            </div>
                            <CardDescription>This information appears on your generated invoices and customer portals.</CardDescription>
                        </CardHeader>
                        <CardContent className="space-y-4">
                            <div className="grid gap-2 sm:grid-cols-2">
                                <div className="space-y-2">
                                    <Label htmlFor="companyName">Company Name</Label>
                                    <Input id="companyName" defaultValue="Billing System Pro LLC" />
                                </div>
                                <div className="space-y-2">
                                    <Label htmlFor="taxId">Tax ID (VAT/GST)</Label>
                                    <Input id="taxId" defaultValue="XX-123456789" />
                                </div>
                                <div className="space-y-2 sm:col-span-2">
                                    <Label htmlFor="address">Billing Address</Label>
                                    <Input id="address" defaultValue="123 Financial District, Suite 400, New York, NY 10004" />
                                </div>
                            </div>
                        </CardContent>

                        <div className="px-6 py-4 border-t bg-muted/20">
                            <h3 className="text-sm font-semibold mb-4">Billing Defaults</h3>
                            <div className="grid gap-4 sm:grid-cols-2 lg:grid-cols-3">
                                <div className="space-y-2">
                                    <Label htmlFor="currency">Default Currency</Label>
                                    <Input id="currency" defaultValue="USD ($)" />
                                </div>
                                <div className="space-y-2">
                                    <Label htmlFor="taxRate">Default Tax Rate (%)</Label>
                                    <Input id="taxRate" defaultValue="8.50" type="number" step="0.01" />
                                </div>
                                <div className="space-y-2">
                                    <Label htmlFor="invoicePrefix">Invoice Prefix</Label>
                                    <Input id="invoicePrefix" defaultValue="INV-2024-" />
                                </div>
                            </div>
                        </div>

                        <CardFooter className="pt-6 border-t flex justify-end">
                            <Button type="submit"><Save className="mr-2 h-4 w-4" /> Save Configuration</Button>
                        </CardFooter>
                    </Card>
                </form>

                <Card>
                    <CardHeader>
                        <CardTitle>C++ Engine Integration</CardTitle>
                        <CardDescription>Configuration for the core billing binary wrapper.</CardDescription>
                    </CardHeader>
                    <CardContent className="space-y-4">
                        <div className="grid gap-4 sm:grid-cols-2">
                            <div className="space-y-2">
                                <Label htmlFor="binaryPath">Core Executable Path</Label>
                                <Input id="binaryPath" defaultValue="./build/make/billing_system" disabled />
                                <p className="text-xs text-muted-foreground mt-1">Read-only path to the compiled C++ engine.</p>
                            </div>
                            <div className="space-y-2">
                                <Label htmlFor="timeout">Timeout (ms)</Label>
                                <Input id="timeout" defaultValue="5000" type="number" />
                            </div>
                        </div>
                        <div className="flex items-center space-x-2 pt-4">
                            <div className="h-2 w-2 rounded-full bg-emerald-500"></div>
                            <p className="text-sm font-medium text-emerald-600 dark:text-emerald-400">Headless Engine Linked & Responding</p>
                        </div>
                    </CardContent>
                </Card>
            </div>
        </div>
    );
};
