//
// Created by jamie on 2025/2/6.
//

#ifndef FAKEDIRECTINPUTDEVICE_H
#define FAKEDIRECTINPUTDEVICE_H

#pragma once

#include "Configs.h"

#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dinput8.lib")

namespace LIBC_NAMESPACE_DECL
{
    namespace SimpleIME
    {
        class FakeDirectInputDevice : public IDirectInputDevice8A
        {
        public:
            FakeDirectInputDevice(IDirectInputDevice8A *device) : m_realDevice(device)
            {
            }

            virtual ~FakeDirectInputDevice() = default;

            /*** IUnknown methods ***/
            STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID *ppvObj)
            {
                return m_realDevice->QueryInterface(riid, ppvObj);
            }

            STDMETHOD_(ULONG, AddRef)(THIS)
            {
                return m_realDevice->AddRef();
            }

            STDMETHOD_(ULONG, Release)(THIS)
            {
                auto result = m_realDevice->Release();
                if (result == 0)
                {
                    delete this;
                }
                return result;
            }

            /*** IDirectInputDevice8W methods ***/
            STDMETHOD(GetCapabilities)(THIS_ LPDIDEVCAPS lpDIDevCaps)
            {
                return m_realDevice->GetCapabilities(lpDIDevCaps);
            }

            STDMETHOD(EnumObjects)(THIS_ LPDIENUMDEVICEOBJECTSCALLBACKA lpCallback, LPVOID pvRef, DWORD dwFlags)
            {
                return m_realDevice->EnumObjects(lpCallback, pvRef, dwFlags);
            }

            STDMETHOD(GetProperty)(THIS_ REFGUID rguidProp, LPDIPROPHEADER pdiph)
            {
                return m_realDevice->GetProperty(rguidProp, pdiph);
            }

            STDMETHOD(SetProperty)(THIS_ REFGUID rguidProp, LPCDIPROPHEADER pdiph)
            {
                return m_realDevice->SetProperty(rguidProp, pdiph);
            }

            STDMETHOD(Acquire)(THIS)
            {
                return m_realDevice->Acquire();
            }

            STDMETHOD(Unacquire)(THIS)
            {
                return m_realDevice->Unacquire();
            }

            STDMETHOD(GetDeviceState)(THIS_ DWORD cbData, LPVOID lpvData)
            {
                return m_realDevice->GetDeviceState(cbData, lpvData);
            }

            STDMETHOD(GetDeviceData)(THIS_ DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut,
                                     DWORD dwFlags)
            {
                return m_realDevice->GetDeviceData(cbObjectData, rgdod, pdwInOut, dwFlags);
            }

            STDMETHOD(SetDataFormat)(THIS_ LPCDIDATAFORMAT lpdf)
            {
                return m_realDevice->SetDataFormat(lpdf);
            }

            STDMETHOD(SetEventNotification)(THIS_ HANDLE hEvent)
            {
                return m_realDevice->SetEventNotification(hEvent);
            }

            STDMETHOD(SetCooperativeLevel)(THIS_ HWND hwnd, DWORD dwFlags)
            {
                dwFlags &= ~(DISCL_EXCLUSIVE | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
                dwFlags |= DISCL_NONEXCLUSIVE;
                return m_realDevice->SetCooperativeLevel(hwnd, dwFlags);
            }

            STDMETHOD(GetObjectInfo)(THIS_ LPDIDEVICEOBJECTINSTANCEA pdidoi, DWORD dwObj, DWORD dwHow)
            {
                return m_realDevice->GetObjectInfo(pdidoi, dwObj, dwHow);
            }

            STDMETHOD(GetDeviceInfo)(THIS_ LPDIDEVICEINSTANCEA pdidi)
            {
                return m_realDevice->GetDeviceInfo(pdidi);
            }

            STDMETHOD(RunControlPanel)(THIS_ HWND hwndOwner, DWORD dwFlags)
            {
                return m_realDevice->RunControlPanel(hwndOwner, dwFlags);
            }

            STDMETHOD(Initialize)(THIS_ HINSTANCE hinst, DWORD dwVersion, REFGUID rguid)
            {
                return m_realDevice->Initialize(hinst, dwVersion, rguid);
            }

            STDMETHOD(CreateEffect)(THIS_ REFGUID rguid, LPCDIEFFECT lpeff, LPDIRECTINPUTEFFECT *ppdeff,
                                    LPUNKNOWN punkOuter)
            {
                return m_realDevice->CreateEffect(rguid, lpeff, ppdeff, punkOuter);
            }

            STDMETHOD(EnumEffects)(THIS_ LPDIENUMEFFECTSCALLBACKA lpCallback, LPVOID pvRef, DWORD dwEffType)
            {
                return m_realDevice->EnumEffects(lpCallback, pvRef, dwEffType);
            }

            STDMETHOD(GetEffectInfo)(THIS_ LPDIEFFECTINFOA pdei, REFGUID rguid)
            {

                return m_realDevice->GetEffectInfo(pdei, rguid);
            }

            STDMETHOD(GetForceFeedbackState)(THIS_ LPDWORD pdwOut)
            {
                return m_realDevice->GetForceFeedbackState(pdwOut);
            }

            STDMETHOD(SendForceFeedbackCommand)(THIS_ DWORD dwFlags)
            {
                return m_realDevice->SendForceFeedbackCommand(dwFlags);
            }

            STDMETHOD(EnumCreatedEffectObjects)(THIS_ LPDIENUMCREATEDEFFECTOBJECTSCALLBACK lpCallback, LPVOID pvRef,
                                                DWORD fl)
            {
                return m_realDevice->EnumCreatedEffectObjects(lpCallback, pvRef, fl);
            }

            STDMETHOD(Escape)(THIS_ LPDIEFFESCAPE pesc)
            {
                return m_realDevice->Escape(pesc);
            }

            STDMETHOD(Poll)(THIS)
            {
                return m_realDevice->Poll();
            }

            STDMETHOD(SendDeviceData)(THIS_ DWORD cbObjectData, LPCDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD fl)
            {
                return m_realDevice->SendDeviceData(cbObjectData, rgdod, pdwInOut, fl);
            }

            STDMETHOD(EnumEffectsInFile)(THIS_ LPCSTR lpszFileName, LPDIENUMEFFECTSINFILECALLBACK pec, LPVOID pvRef,
                                         DWORD dwFlags)
            {
                return m_realDevice->EnumEffectsInFile(lpszFileName, pec, pvRef, dwFlags);
            }

            STDMETHOD(WriteEffectToFile)(THIS_ LPCSTR lpszFileName, DWORD dwEntries, LPDIFILEEFFECT rgDiFileEft,
                                         DWORD dwFlags)
            {
                return m_realDevice->WriteEffectToFile(lpszFileName, dwEntries, rgDiFileEft, dwFlags);
            }

            STDMETHOD(BuildActionMap)(THIS_ LPDIACTIONFORMATA lpdiaf, LPCSTR lpszUserName, DWORD dwFlags)
            {
                return m_realDevice->BuildActionMap(lpdiaf, lpszUserName, dwFlags);
            }

            STDMETHOD(SetActionMap)(THIS_ LPDIACTIONFORMATA lpdiActionFormat, LPCSTR lptszUserName, DWORD dwFlags)
            {
                return m_realDevice->SetActionMap(lpdiActionFormat, lptszUserName, dwFlags);
            }

            STDMETHOD(GetImageInfo)(THIS_ LPDIDEVICEIMAGEINFOHEADERA lpdiDevImageInfoHeader)
            {
                return m_realDevice->GetImageInfo(lpdiDevImageInfoHeader);
            }

        private:
            IDirectInputDevice8A *m_realDevice;
        };

        class FakeDirectInput : public IDirectInput8A
        {
        public:
            explicit FakeDirectInput(IDirectInput8A *obj) : m_realDInput(obj)
            {
            }

            virtual ~FakeDirectInput() = default;

            /*** IUnknown methods ***/
            STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID *ppvObj)
            {
                return m_realDInput->QueryInterface(riid, ppvObj);
            }

            STDMETHOD_(ULONG, AddRef)(THIS)
            {
                return m_realDInput->AddRef();
            }

            STDMETHOD_(ULONG, Release)(THIS)
            {
                auto result = m_realDInput->Release();
                if (result == 0)
                {
                    delete this;
                }
                return result;
            }

            /*** IDirectInput8A methods ***/
            STDMETHOD(CreateDevice)(THIS_ REFGUID rguid, LPDIRECTINPUTDEVICE8A *lplpDirectInputDevice,
                                    LPUNKNOWN pUnkOuter)
            {
                if (rguid == GUID_SysKeyboard)
                {
                    IDirectInputDevice8A *dev;
                    HRESULT               hresult = m_realDInput->CreateDevice(rguid, &dev, pUnkOuter);
                    if (hresult != DI_OK) return hresult;

                    *lplpDirectInputDevice = new FakeDirectInputDevice(dev);
                    return hresult;
                }
                return m_realDInput->CreateDevice(rguid, lplpDirectInputDevice, pUnkOuter);
            }

            STDMETHOD(EnumDevices)(THIS_ DWORD dwDevType, LPDIENUMDEVICESCALLBACKA lpCallback, LPVOID pvRef,
                                   DWORD dwFlags)
            {
                return m_realDInput->EnumDevices(dwDevType, lpCallback, pvRef, dwFlags);
            }

            STDMETHOD(GetDeviceStatus)(THIS_ REFGUID rguidInstance)
            {
                return m_realDInput->GetDeviceStatus(rguidInstance);
            }

            STDMETHOD(RunControlPanel)(THIS_ HWND hwndOwner, DWORD dwFlags)
            {
                return m_realDInput->RunControlPanel(hwndOwner, dwFlags);
            }

            STDMETHOD(Initialize)(THIS_ HINSTANCE hinst, DWORD dwVersion)
            {
                return m_realDInput->Initialize(hinst, dwVersion);
            }

            STDMETHOD(FindDevice)(THIS_ REFGUID rguidClass, LPCSTR ptszName, LPGUID pguidInstance)
            {
                return m_realDInput->FindDevice(rguidClass, ptszName, pguidInstance);
            }

            STDMETHOD(EnumDevicesBySemantics)(THIS_ LPCSTR ptszUserName, LPDIACTIONFORMATA lpdiActionFormat,
                                              LPDIENUMDEVICESBYSEMANTICSCBA lpCallback, LPVOID pvRef, DWORD dwFlags)
            {
                return m_realDInput->EnumDevicesBySemantics(ptszUserName, lpdiActionFormat, lpCallback, pvRef, dwFlags);
            }

            STDMETHOD(ConfigureDevices)(THIS_ LPDICONFIGUREDEVICESCALLBACK lpdiCallback,
                                        LPDICONFIGUREDEVICESPARAMSA lpdiCDParams, DWORD dwFlags, LPVOID pvRefData)
            {
                return m_realDInput->ConfigureDevices(lpdiCallback, lpdiCDParams, dwFlags, pvRefData);
            }

        private:
            IDirectInput8A *m_realDInput;
        };

    } // namespace SimpleIME

} // namespace LIBC_NAMESPACE_DECL

#endif // FAKEDIRECTINPUTDEVICE_H
