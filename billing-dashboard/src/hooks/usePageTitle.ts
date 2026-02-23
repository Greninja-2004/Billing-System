import { useEffect } from 'react';

const BASE_TITLE = 'Billing Pro';

/**
 * Sets the browser tab title for the current page.
 * Also updates the og:title meta tag dynamically.
 * 
 * @example
 * usePageTitle('Invoices'); // → "Invoices | Billing Pro"
 * usePageTitle(); // → "Billing Pro"
 */
export function usePageTitle(pageTitle?: string) {
    useEffect(() => {
        const fullTitle = pageTitle ? `${pageTitle} | ${BASE_TITLE}` : BASE_TITLE;
        document.title = fullTitle;

        // Also update og:title for accuracy
        const ogTitle = document.querySelector('meta[property="og:title"]');
        if (ogTitle) ogTitle.setAttribute('content', fullTitle);

        return () => {
            document.title = BASE_TITLE;
        };
    }, [pageTitle]);
}
