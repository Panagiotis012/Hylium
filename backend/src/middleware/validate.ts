import { z } from 'zod';
import { Request, Response, NextFunction } from 'express';

export const validate = (schema: z.ZodSchema) => {
    return (req: Request, res: Response, next: NextFunction): void => {
        try {
            schema.parse(req.body);
            next();
        } catch (error) {
            if (error instanceof z.ZodError) {
                res.status(400).json({
                    error: 'Validation failed',
                    details: error.errors.map(e => ({
                        field: e.path.join('.'),
                        message: e.message,
                    })),
                });
                return;
            }
            next(error);
        }
    };
};

// Validation schemas
export const schemas = {
    register: z.object({
        email: z.string().email('Invalid email format'),
        password: z.string().min(8, 'Password must be at least 8 characters'),
    }),

    login: z.object({
        email: z.string().email('Invalid email format'),
        password: z.string().min(1, 'Password is required'),
    }),

    newsletter: z.object({
        email: z.string().email('Invalid email format'),
    }),

    contact: z.object({
        email: z.string().email('Invalid email format'),
        subject: z.string().min(1, 'Subject is required').max(200),
        message: z.string().min(10, 'Message must be at least 10 characters').max(5000),
    }),

    download: z.object({
        name: z.string().min(1),
        version: z.string().min(1),
        platform: z.enum(['windows', 'macos', 'linux']),
        url: z.string().url(),
        checksum: z.string().optional(),
    }),

    rpc: z.object({
        method: z.string().min(1),
        params: z.array(z.any()).optional().default([]),
    }),
};
