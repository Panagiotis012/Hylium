import { z } from 'zod';
import { Request, Response, NextFunction } from 'express';
export declare const validate: (schema: z.ZodSchema) => (req: Request, res: Response, next: NextFunction) => void;
export declare const schemas: {
    register: z.ZodObject<{
        email: z.ZodString;
        password: z.ZodString;
    }, "strip", z.ZodTypeAny, {
        email: string;
        password: string;
    }, {
        email: string;
        password: string;
    }>;
    login: z.ZodObject<{
        email: z.ZodString;
        password: z.ZodString;
    }, "strip", z.ZodTypeAny, {
        email: string;
        password: string;
    }, {
        email: string;
        password: string;
    }>;
    newsletter: z.ZodObject<{
        email: z.ZodString;
    }, "strip", z.ZodTypeAny, {
        email: string;
    }, {
        email: string;
    }>;
    contact: z.ZodObject<{
        email: z.ZodString;
        subject: z.ZodString;
        message: z.ZodString;
    }, "strip", z.ZodTypeAny, {
        email: string;
        message: string;
        subject: string;
    }, {
        email: string;
        message: string;
        subject: string;
    }>;
    download: z.ZodObject<{
        name: z.ZodString;
        version: z.ZodString;
        platform: z.ZodEnum<["windows", "macos", "linux"]>;
        url: z.ZodString;
        checksum: z.ZodOptional<z.ZodString>;
    }, "strip", z.ZodTypeAny, {
        name: string;
        version: string;
        platform: "windows" | "macos" | "linux";
        url: string;
        checksum?: string | undefined;
    }, {
        name: string;
        version: string;
        platform: "windows" | "macos" | "linux";
        url: string;
        checksum?: string | undefined;
    }>;
    rpc: z.ZodObject<{
        method: z.ZodString;
        params: z.ZodDefault<z.ZodOptional<z.ZodArray<z.ZodAny, "many">>>;
    }, "strip", z.ZodTypeAny, {
        params: any[];
        method: string;
    }, {
        method: string;
        params?: any[] | undefined;
    }>;
};
//# sourceMappingURL=validate.d.ts.map