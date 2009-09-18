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

package android.bluetooth;

/**
 * System private API for Bluetooth A2DP service
 *
 * {@hide}
 */
interface IBluetoothA2dp {
    int connectSink(in String address);
    int disconnectSink(in String address);
    List<String> listConnectedSinks();
    int getSinkState(in String address);
    int setSinkPriority(in String address, int priority);
    int getSinkPriority(in String address);
}
