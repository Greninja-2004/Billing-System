import { useEffect, useState } from 'react';
import { useSearchParams, useNavigate } from 'react-router-dom';
import { Card, CardHeader, CardTitle, CardDescription, CardContent, CardFooter } from '../components/ui/card';
import { Button } from '../components/ui/button';
import { CheckCircle2, XCircle, ArrowRight, Loader2 } from 'lucide-react';

export const Success = () => {
    const [searchParams] = useSearchParams();
    const navigate = useNavigate();
    const [status, setStatus] = useState<'loading' | 'success' | 'invalid'>('loading');

    // In a real app, you would verify the session_id with your backend
    const sessionId = searchParams.get('session_id');

    useEffect(() => {
        if (sessionId) {
            // Mocking verification delay
            setTimeout(() => setStatus('success'), 1500);
        } else {
            setStatus('invalid');
        }
    }, [sessionId]);

    return (
        <div className="min-h-screen bg-muted/20 flex flex-col items-center justify-center p-4">
            <Card className="w-full max-w-md shadow-lg border-t-4 border-t-emerald-500">
                <CardHeader className="text-center pt-8 pb-4">
                    <div className="mx-auto flex h-16 w-16 items-center justify-center rounded-full bg-emerald-100 mb-4">
                        {status === 'loading' ? (
                            <Loader2 className="h-8 w-8 text-emerald-600 animate-spin" />
                        ) : status === 'success' ? (
                            <CheckCircle2 className="h-10 w-10 text-emerald-600" />
                        ) : (
                            <XCircle className="h-10 w-10 text-destructive" />
                        )}
                    </div>
                    <CardTitle className="text-2xl font-bold tracking-tight">
                        {status === 'loading' ? 'Verifying Payment...' :
                            status === 'success' ? 'Payment Successful!' : 'Invalid Session'}
                    </CardTitle>
                    <CardDescription className="text-base mt-2">
                        {status === 'loading' ? 'Please wait while we confirm your transaction with Stripe.' :
                            status === 'success' ? 'Your invoice has been paid. A receipt has been sent to your email.' : 'Try initiating the payment again from the dashboard.'}
                    </CardDescription>
                </CardHeader>

                {status === 'success' && (
                    <CardContent className="bg-muted/30 p-4 m-4 rounded-lg flex justify-between items-center text-sm border border-border/50">
                        <span className="text-muted-foreground font-medium">Transaction ID</span>
                        <span className="font-mono text-xs max-w-[150px] truncate">{sessionId}</span>
                    </CardContent>
                )}

                <CardFooter className="flex flex-col space-y-2 pt-2 pb-8">
                    <Button
                        className="w-full h-11"
                        onClick={() => navigate('/invoices')}
                        disabled={status === 'loading'}
                    >
                        Return to Dashboard <ArrowRight className="ml-2 h-4 w-4" />
                    </Button>
                </CardFooter>
            </Card>
        </div>
    );
};
