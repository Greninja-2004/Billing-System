import { usePageTitle } from '../hooks/usePageTitle';
import { apiFetch } from '../utils/api';
import {  } from "../config";
import { useState } from 'react';
import { Card, CardContent } from '../components/ui/card';
import { Button } from '../components/ui/button';

declare const Razorpay: any;
import {
    Check, Zap, Crown, Building2, Loader2, CreditCard,
    Shield, TrendingUp, Users, FileText, Star
} from 'lucide-react';

const PLANS = [
    {
        id: 'starter',
        name: 'Starter',
        price: 9,
        period: 'month',
        description: 'Perfect for freelancers and solo businesses',
        icon: Zap,
        color: 'text-blue-500',
        bg: 'bg-blue-500/10',
        border: 'border-blue-500/20',
        highlight: false,
        features: [
            'Up to 50 invoices/month',
            '5 customers',
            'Basic payment collection',
            'PDF invoice downloads',
            'Email support',
        ],
        unavailable: ['Team collaboration', 'Advanced analytics', 'API access', 'Custom branding'],
    },
    {
        id: 'professional',
        name: 'Professional',
        price: 29,
        period: 'month',
        description: 'For growing teams with advanced needs',
        icon: Crown,
        color: 'text-purple-500',
        bg: 'bg-purple-500/10',
        border: 'border-purple-500/30',
        highlight: true,
        badge: 'Most Popular',
        features: [
            'Unlimited invoices',
            'Unlimited customers',
            'Stripe payment integration',
            'Automated reminders',
            'Advanced analytics & reports',
            'Team collaboration (5 seats)',
            'Priority email + chat support',
        ],
        unavailable: ['API access', 'Custom branding'],
    },
    {
        id: 'enterprise',
        name: 'Enterprise',
        price: 79,
        period: 'month',
        description: 'Full-scale solution for large organizations',
        icon: Building2,
        color: 'text-amber-500',
        bg: 'bg-amber-500/10',
        border: 'border-amber-500/20',
        highlight: false,
        features: [
            'Everything in Professional',
            'Unlimited team seats',
            'Full REST API access',
            'Custom branding & white-label',
            'Dedicated account manager',
            'SSO / SAML',
            '99.99% SLA uptime guarantee',
            'Custom integrations',
        ],
        unavailable: [],
    },
];

const CURRENT_PLAN = 'professional'; // Simulated current plan

export const Subscriptions = () => {
    usePageTitle('Subscriptions');
    const [selectedBilling, setSelectedBilling] = useState<'month' | 'year'>('month');
    const [processingPlanId, setProcessingPlanId] = useState<string | null>(null);

    const getPrice = (base: number) =>
        selectedBilling === 'year' ? Math.round(base * 0.8) : base;

    const handleSubscribe = async (plan: typeof PLANS[0]) => {
        if (plan.id === CURRENT_PLAN) return;
        try {
            setProcessingPlanId(plan.id);

            // Step 1: Create Razorpay order
            const res = await apiFetch(`/api/create-razorpay-order`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({
                    invoiceId: `sub_${plan.id}`,
                    amount: getPrice(plan.price),
                    customerName: `${plan.name} Plan Subscription`,
                }),
            });
            const order = await res.json();

            if (order.error) {
                alert('Failed to create subscription order. Please try again.');
                setProcessingPlanId(null);
                return;
            }

            // Step 2: Open Razorpay modal
            const options = {
                key: order.keyId,
                amount: order.amount,
                currency: order.currency,
                name: 'Billing Pro',
                description: `${plan.name} Plan - ${selectedBilling === 'year' ? 'Annual' : 'Monthly'}`,
                order_id: order.orderId,
                handler: async (response: any) => {
                    // Step 3: Verify
                    const verify = await apiFetch(`/api/verify-razorpay-payment`, {
                        method: 'POST',
                        headers: { 'Content-Type': 'application/json' },
                        body: JSON.stringify({
                            razorpay_order_id: response.razorpay_order_id,
                            razorpay_payment_id: response.razorpay_payment_id,
                            razorpay_signature: response.razorpay_signature,
                        }),
                    });
                    const result = await verify.json();
                    if (result.success) {
                        alert(`✅ Subscribed to ${plan.name}! Payment ID: ${response.razorpay_payment_id}`);
                    } else {
                        alert('⚠️ Payment verification failed. Contact support.');
                    }
                    setProcessingPlanId(null);
                },
                theme: { color: '#3b82f6' },
                modal: { ondismiss: () => setProcessingPlanId(null) },
            };

            const rzp = new Razorpay(options);
            rzp.open();

        } catch (err) {
            console.error(err);
            alert('An error occurred. Please try again.');
            setProcessingPlanId(null);
        }
    };

    return (
        <div className="space-y-8 pb-8">
            {/* Header */}
            <div className="animate-slide-up">
                <h1 className="text-3xl font-bold tracking-tight">Subscriptions</h1>
                <p className="text-muted-foreground mt-1">
                    Choose the plan that fits your business. Upgrade or downgrade anytime.
                </p>
            </div>

            {/* Current Plan Banner */}
            <Card className="border-primary/30 bg-primary/5 animate-slide-up" style={{ animationDelay: '60ms' }}>
                <CardContent className="flex flex-col sm:flex-row items-start sm:items-center justify-between gap-4 pt-6">
                    <div className="flex items-center gap-3">
                        <div className="h-10 w-10 rounded-xl bg-purple-500/10 flex items-center justify-center">
                            <Crown className="h-5 w-5 text-purple-500" />
                        </div>
                        <div>
                            <p className="font-semibold">Currently on <span className="text-primary">Professional Plan</span></p>
                            <p className="text-sm text-muted-foreground">Renews on March 23, 2026 · $29/month</p>
                        </div>
                    </div>
                    <div className="flex gap-2 flex-wrap">
                        <Button variant="outline" size="sm">Manage Billing</Button>
                        <Button variant="outline" size="sm" className="text-destructive border-destructive/30 hover:bg-destructive/5">
                            Cancel Plan
                        </Button>
                    </div>
                </CardContent>
            </Card>

            {/* Billing Toggle */}
            <div className="flex justify-center animate-slide-up" style={{ animationDelay: '100ms' }}>
                <div className="inline-flex items-center bg-muted rounded-xl p-1 gap-1">
                    <button
                        onClick={() => setSelectedBilling('month')}
                        className={`px-5 py-2 rounded-lg text-sm font-medium transition-all ${selectedBilling === 'month' ? 'bg-background shadow text-foreground' : 'text-muted-foreground hover:text-foreground'}`}
                    >
                        Monthly
                    </button>
                    <button
                        onClick={() => setSelectedBilling('year')}
                        className={`px-5 py-2 rounded-lg text-sm font-medium transition-all flex items-center gap-2 ${selectedBilling === 'year' ? 'bg-background shadow text-foreground' : 'text-muted-foreground hover:text-foreground'}`}
                    >
                        Annual
                        <span className="bg-emerald-500/10 text-emerald-600 text-xs font-bold px-1.5 py-0.5 rounded-full">
                            Save 20%
                        </span>
                    </button>
                </div>
            </div>

            {/* Plan Cards */}
            <div className="grid md:grid-cols-3 gap-6 stagger animate-slide-up" style={{ animationDelay: '140ms' }}>
                {PLANS.map((plan) => {
                    const Icon = plan.icon;
                    const isCurrent = plan.id === CURRENT_PLAN;
                    const isProcessing = processingPlanId === plan.id;

                    return (
                        <div
                            key={plan.id}
                            className={`relative rounded-2xl border-2 p-6 flex flex-col gap-5 transition-all duration-300 hover-lift ${plan.highlight
                                ? 'border-primary shadow-xl shadow-primary/15 bg-primary/5'
                                : `${plan.border} bg-card`
                                }`}
                        >
                            {plan.badge && (
                                <div className="absolute -top-3 left-1/2 -translate-x-1/2">
                                    <span className="bg-primary text-primary-foreground text-xs font-bold px-3 py-1 rounded-full flex items-center gap-1">
                                        <Star className="h-3 w-3" /> {plan.badge}
                                    </span>
                                </div>
                            )}

                            {/* Plan Header */}
                            <div>
                                <div className={`h-11 w-11 rounded-xl ${plan.bg} flex items-center justify-center mb-4`}>
                                    <Icon className={`h-5 w-5 ${plan.color}`} />
                                </div>
                                <h3 className="text-xl font-bold">{plan.name}</h3>
                                <p className="text-sm text-muted-foreground mt-1">{plan.description}</p>
                            </div>

                            {/* Price */}
                            <div className="flex items-end gap-1">
                                <span className="text-4xl font-extrabold tracking-tight">${getPrice(plan.price)}</span>
                                <span className="text-muted-foreground text-sm pb-1">/mo</span>
                                {selectedBilling === 'year' && (
                                    <span className="ml-2 text-xs text-muted-foreground line-through pb-1">${plan.price}</span>
                                )}
                            </div>
                            {selectedBilling === 'year' && (
                                <p className="text-xs text-emerald-600 font-medium -mt-3">
                                    Billed ${getPrice(plan.price) * 12}/year
                                </p>
                            )}

                            {/* CTA Button */}
                            <Button
                                className={`w-full ${plan.highlight ? '' : 'variant-outline'}`}
                                variant={plan.highlight ? 'default' : 'outline'}
                                disabled={isCurrent || isProcessing}
                                onClick={() => handleSubscribe(plan)}
                            >
                                {isProcessing ? (
                                    <><Loader2 className="mr-2 h-4 w-4 animate-spin" /> Processing...</>
                                ) : isCurrent ? (
                                    <><Check className="mr-2 h-4 w-4" /> Current Plan</>
                                ) : (
                                    <><CreditCard className="mr-2 h-4 w-4" /> Subscribe</>
                                )}
                            </Button>

                            {/* Features */}
                            <div className="space-y-2.5 flex-1">
                                {plan.features.map((f) => (
                                    <div key={f} className="flex items-start gap-2 text-sm">
                                        <Check className="h-4 w-4 text-emerald-500 mt-0.5 flex-shrink-0" />
                                        <span>{f}</span>
                                    </div>
                                ))}
                                {plan.unavailable?.map((f) => (
                                    <div key={f} className="flex items-start gap-2 text-sm text-muted-foreground/50">
                                        <span className="h-4 w-4 mt-0.5 flex-shrink-0 text-center text-xs">✕</span>
                                        <span className="line-through">{f}</span>
                                    </div>
                                ))}
                            </div>
                        </div>
                    );
                })}
            </div>

            {/* Feature Comparison Cards */}
            <div className="animate-slide-up" style={{ animationDelay: '280ms' }}>
                <h2 className="text-xl font-bold mb-4">What's included in every plan</h2>
                <div className="grid sm:grid-cols-2 lg:grid-cols-4 gap-4">
                    {[
                        { icon: Shield, label: 'Bank-level Security', desc: 'TLS 1.3 + AES-256 encryption' },
                        { icon: TrendingUp, label: 'Real-time Analytics', desc: 'Live revenue charts and reports' },
                        { icon: Users, label: 'Multi-user Access', desc: 'Granular role-based permissions' },
                        { icon: FileText, label: 'Automated Invoicing', desc: 'Recurring billing & PDF exports' },
                    ].map(({ icon: Icon, label, desc }) => (
                        <Card key={label} className="p-4 hover-lift">
                            <div className="flex items-start gap-3">
                                <div className="h-9 w-9 rounded-lg bg-primary/10 flex items-center justify-center flex-shrink-0">
                                    <Icon className="h-4 w-4 text-primary" />
                                </div>
                                <div>
                                    <p className="text-sm font-semibold">{label}</p>
                                    <p className="text-xs text-muted-foreground mt-0.5">{desc}</p>
                                </div>
                            </div>
                        </Card>
                    ))}
                </div>
            </div>
        </div>
    );
};
