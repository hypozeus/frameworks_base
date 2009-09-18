/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package android.text.util;

/**
 * This class stores an RFC 822-like name, address, and comment,
 * and provides methods to convert them to quoted strings.
 */
public class Rfc822Token {
    private String mName, mAddress, mComment;

    /**
     * Creates a new Rfc822Token with the specified name, address,
     * and comment.
     */
    public Rfc822Token(String name, String address, String comment) {
        mName = name;
        mAddress = address;
        mComment = comment;
    }

    /**
     * Returns the name part.
     */
    public String getName() {
        return mName;
    }

    /**
     * Returns the address part.
     */
    public String getAddress() {
        return mAddress;
    }

    /**
     * Returns the comment part.
     */
    public String getComment() {
        return mComment;
    }

    /**
     * Changes the name to the specified name.
     */
    public void setName(String name) {
        mName = name;
    }

    /**
     * Changes the address to the specified address.
     */
    public void setAddress(String address) {
        mAddress = address;
    }

    /**
     * Changes the comment to the specified comment.
     */
    public void setComment(String comment) {
        mComment = comment;
    }

    /**
     * Returns the name (with quoting added if necessary),
     * the comment (in parentheses), and the address (in angle brackets).
     * This should be suitable for inclusion in an RFC 822 address list.
     */
    public String toString() {
        StringBuilder sb = new StringBuilder();

        if (mName != null && mName.length() != 0) {
            sb.append(quoteNameIfNecessary(mName));
            sb.append(' ');
        }

        if (mComment != null && mComment.length() != 0) {
            sb.append('(');
            sb.append(quoteComment(mComment));
            sb.append(") ");
        }

        if (mAddress != null && mAddress.length() != 0) {
            sb.append('<');
            sb.append(mAddress);
            sb.append('>');
        }

        return sb.toString();
    }

    /**
     * Returns the name, conservatively quoting it if there are any
     * characters that are likely to cause trouble outside of a
     * quoted string, or returning it literally if it seems safe.
     */
    public static String quoteNameIfNecessary(String name) {
        int len = name.length();

        for (int i = 0; i < len; i++) {
            char c = name.charAt(i);

            if (! ((c >= 'A' && c <= 'Z') ||
                   (c >= 'a' && c <= 'z') ||
                   (c == ' ') ||
                   (c >= '0' && c <= '9'))) {
                return '"' + quoteName(name) + '"';
            }
        }

        return name;
    }

    /**
     * Returns the name, with internal backslashes and quotation marks
     * preceded by backslashes.  The outer quote marks themselves are not
     * added by this method.
     */
    public static String quoteName(String name) {
        StringBuilder sb = new StringBuilder();

        int len = name.length();
        for (int i = 0; i < len; i++) {
            char c = name.charAt(i);

            if (c == '\\' || c == '"') {
                sb.append('\\');
            }

            sb.append(c);
        }

        return sb.toString();
    }

    /**
     * Returns the comment, with internal backslashes and parentheses
     * preceded by backslashes.  The outer parentheses themselves are
     * not added by this method.
     */
    public static String quoteComment(String comment) {
        int len = comment.length();
        StringBuilder sb = new StringBuilder();

        for (int i = 0; i < len; i++) {
            char c = comment.charAt(i);

            if (c == '(' || c == ')' || c == '\\') {
                sb.append('\\');
            }

            sb.append(c);
        }

        return sb.toString();
    }
}

