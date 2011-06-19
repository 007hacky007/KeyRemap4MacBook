#include "base.hpp"
#include "Config.hpp"
#include "EventOutputQueue.hpp"
#include "FlagStatus.hpp"
#include "VirtualKey.hpp"
#include "VK_JIS_IM_CHANGE.hpp"
#include "VK_JIS_TEMPORARY.hpp"

namespace org_pqrs_KeyRemap4MacBook {
  BridgeWorkSpaceData VirtualKey::VK_JIS_IM_CHANGE::wsd_public_;
  BridgeWorkSpaceData VirtualKey::VK_JIS_IM_CHANGE::wsd_save_[VirtualKey::VK_JIS_IM_CHANGE::SavedInputModeIndex::MAX + 1];
  BridgeWorkSpaceData VirtualKey::VK_JIS_IM_CHANGE::wsd_learned_;

  KeyCode VirtualKey::VK_JIS_IM_CHANGE::newkeycode_;
  Flags VirtualKey::VK_JIS_IM_CHANGE::newflag_;
  TimerWrapper VirtualKey::VK_JIS_IM_CHANGE::restore_timer_;
  VirtualKey::VK_JIS_IM_CHANGE::CallbackType::Value VirtualKey::VK_JIS_IM_CHANGE::callbacktype_ = VirtualKey::VK_JIS_IM_CHANGE::CallbackType::INIT;
  bool VirtualKey::VK_JIS_IM_CHANGE::omit_initialize_ = true;
  // XXX change variable name
  int VirtualKey::VK_JIS_IM_CHANGE::case1_pass_restore2_ = 0;

  VirtualKey::VK_JIS_IM_CHANGE::SavedInputModeIndex::Value VirtualKey::VK_JIS_IM_CHANGE::pre_index2_    = VirtualKey::VK_JIS_IM_CHANGE::SavedInputModeIndex::NONE;
  VirtualKey::VK_JIS_IM_CHANGE::SavedInputModeIndex::Value VirtualKey::VK_JIS_IM_CHANGE::cur_index2_    = VirtualKey::VK_JIS_IM_CHANGE::SavedInputModeIndex::NONE;
  VirtualKey::VK_JIS_IM_CHANGE::SavedInputModeIndex::Value VirtualKey::VK_JIS_IM_CHANGE::others_index2_ = VirtualKey::VK_JIS_IM_CHANGE::SavedInputModeIndex::NONE;

  int VirtualKey::VK_JIS_IM_CHANGE::sign_plus_minus2_ = -99;
  int VirtualKey::VK_JIS_IM_CHANGE::counter_plus_minus2_ = 0;
  int VirtualKey::VK_JIS_IM_CHANGE::pre_counter_plus_minus2_ = 0;
  bool VirtualKey::VK_JIS_IM_CHANGE::seesaw_init2_ = false;

  void
  VirtualKey::VK_JIS_IM_CHANGE::initialize(IOWorkLoop& workloop)
  {
    restore_timer_.initialize(&workloop, NULL, restore_timer_callback);
  }

  void
  VirtualKey::VK_JIS_IM_CHANGE::terminate(void)
  {
    restore_timer_.terminate();
  }

  void
  VirtualKey::VK_JIS_IM_CHANGE::scheduleCallback(VirtualKey::VK_JIS_IM_CHANGE::CallbackType::Value callbacktype)
  {
    callbacktype_  = callbacktype;
    restore_timer_.setTimeoutMS(Config::get_japanese_keyinterval_timeout());
  }

  void
  VirtualKey::VK_JIS_IM_CHANGE::cancelCallback(void)
  {
    restore_timer_.cancelTimeout();
  }

  void
  VirtualKey::VK_JIS_IM_CHANGE::restore_timer_callback(OSObject* owner, IOTimerEventSource* sender)
  {
    switch (callbacktype_) {
      case VirtualKey::VK_JIS_IM_CHANGE::CallbackType::INIT:
        omit_initialize_ = false;
        break;

      case VirtualKey::VK_JIS_IM_CHANGE::CallbackType::RESTORE:
        EventOutputQueue::FireKey::fire_downup(Flags(0), KeyCode::VK_JIS_TEMPORARY_RESTORE, CommonData::getcurrent_keyboardType());
        omit_initialize_ = true;
        break;

      case VirtualKey::VK_JIS_IM_CHANGE::CallbackType::SEESAW_INIT:
        seesaw_init2_ = true;
        break;
    }
  }

  void
  VirtualKey::VK_JIS_IM_CHANGE::static_set_case1_pass_restore(int case1_pass_restore00)
  {
    int ignore_improveIM = Config::get_essential_config(BRIDGE_ESSENTIAL_CONFIG_INDEX_remap_jis_ignore_improvement_IM_changing);
    if (ignore_improveIM || case1_pass_restore00 != 0 && case1_pass_restore00 != 1) return;
    case1_pass_restore2_ = case1_pass_restore00;
  }

  bool
  VirtualKey::VK_JIS_IM_CHANGE::handle(const Params_KeyboardEventCallBack& params)
  {
    int ignore_improveIM = Config::get_essential_config(BRIDGE_ESSENTIAL_CONFIG_INDEX_remap_jis_ignore_improvement_IM_changing);
    int use_ainu = Config::get_essential_config(BRIDGE_ESSENTIAL_CONFIG_INDEX_general_use_ainu);
    KeyCode key00  = params.key;
    SavedInputModeIndex::Value index = SavedInputModeIndex::NONE;
    int skip00[SavedInputModeIndex::MAX + 1] = { 0 };
    ReplaceType::Value replacetype = ReplaceType::NONE;
    SeesawType::Value seesawType = SeesawType::NONE;
    SkipType::Value skipType = SkipType::NONE;

    if (key00 == KeyCode::VK_JIS_IM_CHANGE_CUR_PRE ||
        key00 == KeyCode::VK_JIS_IM_CHANGE_SKIP_NONE_FORWARD  ||
        key00 == KeyCode::VK_JIS_IM_CHANGE_SKIP_NONE_BACK ||
        key00 == KeyCode::VK_JIS_IM_CHANGE_SKIP_PRE_FORWARD ||
        key00 == KeyCode::VK_JIS_IM_CHANGE_SKIP_PRE_BACK ||

        key00 == KeyCode::VK_JIS_IM_CHANGE_EISUU_KANA ||
        key00 == KeyCode::VK_JIS_IM_CHANGE_EISUU_OTHERS ||
        key00 == KeyCode::VK_JIS_IM_CHANGE_KANA_EISUU ||
        key00 == KeyCode::VK_JIS_IM_CHANGE_KANA_OTHERS ||
        key00 == KeyCode::VK_JIS_IM_CHANGE_SKIP_KANA_EISUU ||
        key00 == KeyCode::VK_JIS_IM_CHANGE_SKIP_EISUU ||
        key00 == KeyCode::VK_JIS_IM_CHANGE_SKIP_KANA) {} else {
      return false;
    }

    if (params.ex_iskeydown) {
      if (key00 == KeyCode::VK_JIS_IM_CHANGE_CUR_PRE) {
        seesawType = SeesawType::CUR_PRE;
      } else if (key00 == KeyCode::VK_JIS_IM_CHANGE_EISUU_KANA) {
        seesawType = SeesawType::EISUU_KANA;
      } else if (key00 == KeyCode::VK_JIS_IM_CHANGE_KANA_OTHERS) {
        seesawType = SeesawType::KANA_OTHERS;
      } else if (key00 == KeyCode::VK_JIS_IM_CHANGE_KANA_EISUU) {
        seesawType = SeesawType::KANA_EISUU;
      } else if (key00 == KeyCode::VK_JIS_IM_CHANGE_EISUU_OTHERS) {
        seesawType = SeesawType::EISUU_OTHERS;
      } else if (key00 == KeyCode::VK_JIS_IM_CHANGE_SKIP_NONE_FORWARD) {
        skipType = SkipType::NONE_FORWARD;
      } else if (key00 == KeyCode::VK_JIS_IM_CHANGE_SKIP_PRE_FORWARD) {
        skipType = SkipType::PRE_FORWARD;
      } else if (key00 == KeyCode::VK_JIS_IM_CHANGE_SKIP_NONE_BACK) {
        skipType = SkipType::NONE_BACK;
      } else if (key00 == KeyCode::VK_JIS_IM_CHANGE_SKIP_PRE_BACK) {
        skipType = SkipType::PRE_BACK;
      } else if (key00 == KeyCode::VK_JIS_IM_CHANGE_SKIP_KANA_EISUU) {
        skipType = SkipType::EISUU_KANA;
      } else if (key00 == KeyCode::VK_JIS_IM_CHANGE_SKIP_KANA) {
        skipType = SkipType::KANA;
      } else if (key00 == KeyCode::VK_JIS_IM_CHANGE_SKIP_EISUU) {
        skipType = SkipType::EISUU;
      }

      newflag_ = ModifierFlag::NONE;
      if (seesawType != SeesawType::NONE) {
        index = VirtualKey::VK_JIS_IM_CHANGE::get_index_for_seesaw_AtoB_WSD(seesawType);

        if (seesawType == SeesawType::EISUU_KANA   && index == SavedInputModeIndex::EISU ||
            seesawType == SeesawType::KANA_OTHERS  && index == SavedInputModeIndex::HIRA ||
            seesawType == SeesawType::KANA_EISUU   && index == SavedInputModeIndex::HIRA ||
            seesawType == SeesawType::EISUU_OTHERS && index == SavedInputModeIndex::EISU) {
          VirtualKey::VK_JIS_IM_CHANGE::scheduleCallback(VirtualKey::VK_JIS_IM_CHANGE::CallbackType::SEESAW_INIT);
        }

      } else {
        int sign00;
        if (skipType == SkipType::EISUU_KANA ||
            skipType == SkipType::KANA ||
            skipType == SkipType::EISUU) {
          VirtualKey::VK_JIS_IM_CHANGE::reverse_sign_CHANGE_SKIP(9);
        }

        if (skipType == SkipType::NONE_BACK ||
            skipType == SkipType::PRE_BACK) {
          sign00 = -1;
        } else {
          sign00 =  1;
        }

        if (! use_ainu) {
          skip00[SavedInputModeIndex::AINU] = 1;
        }
        if (skipType == SkipType::NONE_FORWARD ||
            skipType == SkipType::NONE_BACK) {
          replacetype = ReplaceType::NOSKIP;
        } else {
          if (skipType == SkipType::EISUU_KANA) {
            skip00[SavedInputModeIndex::EISU] = 1;
            skip00[SavedInputModeIndex::HIRA] = 1;
            replacetype = ReplaceType::SKIP_SPECIFIC;
          } else if (skipType == SkipType::KANA) {
            skip00[SavedInputModeIndex::HIRA] = 1;
            replacetype = ReplaceType::SKIP_SPECIFIC;
          } else if (skipType == SkipType::EISUU) {
            skip00[SavedInputModeIndex::EISU] = 1;
            replacetype = ReplaceType::SKIP_SPECIFIC;
          } else {
            replacetype = ReplaceType::SKIP_PREVIOUS;
          }
        }
        index = VirtualKey::VK_JIS_IM_CHANGE::get_index_for_replaceWSD(sign00, skip00, replacetype);
      }

      if (index == SavedInputModeIndex::EISU) {
        newkeycode_ = KeyCode::JIS_COLON;
        newflag_    = ModifierFlag::CONTROL_L | ModifierFlag::SHIFT_L;
      } else if (index == SavedInputModeIndex::HIRA) {
        newkeycode_ = KeyCode::J;
        newflag_    = ModifierFlag::CONTROL_L | ModifierFlag::SHIFT_L;
      } else if (index == SavedInputModeIndex::KATA) {
        newkeycode_ = KeyCode::K;
        newflag_    = ModifierFlag::CONTROL_L | ModifierFlag::SHIFT_L;
      } else if (index == SavedInputModeIndex::HKAT) {
        newkeycode_ = KeyCode::SEMICOLON;
        newflag_    = ModifierFlag::CONTROL_L | ModifierFlag::SHIFT_L;
      } else if (index == SavedInputModeIndex::FEIS) {
        newkeycode_ = KeyCode::L;
        newflag_    = ModifierFlag::CONTROL_L | ModifierFlag::SHIFT_L;
      } else if (index == SavedInputModeIndex::AINU) {
        newkeycode_ = KeyCode::JIS_BRACKET_RIGHT;
        newflag_    = ModifierFlag::CONTROL_L | ModifierFlag::SHIFT_L;
      } else {
        return true;
      }
    }

    if (params.ex_iskeydown) {
      FlagStatus::temporary_decrease(params.flags);
      FlagStatus::temporary_increase(newflag_);
    } else {
      FlagStatus::temporary_decrease(newflag_);
      FlagStatus::temporary_increase(params.flags);
    }

    if (params.ex_iskeydown) {
      bool result00 = false;
      if (! ignore_improveIM) {
        result00 = VirtualKey::VK_JIS_IM_CHANGE::replace_WSD(newkeycode_, newflag_);
      }
      if (result00) {
        omit_initialize_ = true;
      } else {
        omit_initialize_ = false;
      }
    }

    Params_KeyboardEventCallBack::auto_ptr ptr(Params_KeyboardEventCallBack::alloc(params.eventType,
                                                                                   FlagStatus::makeFlags(),
                                                                                   newkeycode_,
                                                                                   params.keyboardType,
                                                                                   params.repeat));
    if (ptr) {
      EventOutputQueue::FireKey::fire(*ptr);
    }
    return true;
  }

  void
  VirtualKey::VK_JIS_IM_CHANGE::ControlWorkspaceData(Params_KeyboardEventCallBack& params, StageType::Value stage)
  {
    bool isKeyDown = params.ex_iskeydown;
    bool keyisCCOS;
    bool isonCCO;
    bool isonS;
    bool is_eisuu_kana_kanashift;
    bool isonCCOSonly;
    int ignore_improveIM = Config::get_essential_config(BRIDGE_ESSENTIAL_CONFIG_INDEX_remap_jis_ignore_improvement_IM_changing);
    bool result00;

    if (ignore_improveIM) return;

    keyisCCOS = params.key == KeyCode::COMMAND_L || params.key == KeyCode::COMMAND_R ||
                params.key == KeyCode::CONTROL_L || params.key == KeyCode::CONTROL_R ||
                params.key == KeyCode::OPTION_L  || params.key == KeyCode::OPTION_R  ||
                params.key == KeyCode::SHIFT_L   || params.key == KeyCode::SHIFT_R;

    isonCCO = FlagStatus::makeFlags().isOn(ModifierFlag::COMMAND_L) || params.flags == ModifierFlag::COMMAND_L ||
              FlagStatus::makeFlags().isOn(ModifierFlag::COMMAND_R) || params.flags == ModifierFlag::COMMAND_R ||
              FlagStatus::makeFlags().isOn(ModifierFlag::CONTROL_L) || params.flags == ModifierFlag::CONTROL_L ||
              FlagStatus::makeFlags().isOn(ModifierFlag::CONTROL_R) || params.flags == ModifierFlag::CONTROL_R ||
              FlagStatus::makeFlags().isOn(ModifierFlag::OPTION_L)  || params.flags == ModifierFlag::OPTION_L  ||
              FlagStatus::makeFlags().isOn(ModifierFlag::OPTION_R)  || params.flags == ModifierFlag::OPTION_R;

    isonS = FlagStatus::makeFlags().isOn(ModifierFlag::SHIFT_L) || params.flags == ModifierFlag::SHIFT_L ||
            FlagStatus::makeFlags().isOn(ModifierFlag::SHIFT_R) || params.flags == ModifierFlag::SHIFT_R;

    is_eisuu_kana_kanashift =  params.key == KeyCode::JIS_EISUU && ! isonCCO && ! isonS ||
                              params.key == KeyCode::JIS_EISUU && ! isonS ||
                              params.key == KeyCode::JIS_KANA  && ! isonCCO && ! isonS ||
                              params.key == KeyCode::JIS_KANA  && isonS;

    isonCCOSonly = (isonS || isonCCO) && ! keyisCCOS;

    bool isPPP = (is_eisuu_kana_kanashift || isonCCOSonly);
    KeyCode key00  = params.key;
    Flags flag00 = params.flags;

    if (isKeyDown) {
      if (stage == StageType::POST_REMAP) {
        VirtualKey::VK_JIS_IM_CHANGE::reverse_sign_CHANGE_SKIP(0);
      } else if (stage == StageType::JUST_AFTER_REMAP) {
        VirtualKey::VK_JIS_IM_CHANGE::reverse_sign_CHANGE_SKIP(1);
        return;
      }
    }

    if (! isKeyDown) {
      if (! isPPP && ! isonCCO && stage == StageType::POST_REMAP) {
        VirtualKey::VK_JIS_IM_CHANGE::scheduleCallback(VirtualKey::VK_JIS_IM_CHANGE::CallbackType::INIT);
      }

      return;
    }

    if (stage == StageType::POST_REMAP) {
      VirtualKey::VK_JIS_IM_CHANGE::cancelCallback();
    }

    if (stage == StageType::POST_REMAP && ! ignore_improveIM) {
      VirtualKey::VK_JIS_IM_CHANGE::learn_WSD();
    }

    bool conC2_1 =   isPPP && stage == StageType::NON_REMAPPED;
    bool conC2_2 = ! isPPP && stage == StageType::POST_REMAP && (! omit_initialize_);
    bool conC3   = ! isPPP && stage == StageType::NON_REMAPPED;

    if (ignore_improveIM) {
      if (conC2_1 || conC2_2) {
        VirtualKey::VK_JIS_IM_CHANGE::update_WSD();
        omit_initialize_ = ! conC2_1;
        VirtualKey::VK_JIS_TEMPORARY::resetSavedIMD();
      }
    } else {
      if (conC2_1) {
        result00 = VirtualKey::VK_JIS_IM_CHANGE::replace_WSD(key00, flag00);
        if (result00) {
          omit_initialize_ = true;
          VirtualKey::VK_JIS_TEMPORARY::resetSavedIMD();
        } else {
          omit_initialize_ = false;
        }
      }else if (conC2_2) {
        VirtualKey::VK_JIS_IM_CHANGE::update_WSD();
        omit_initialize_ = true;
        VirtualKey::VK_JIS_TEMPORARY::resetSavedIMD();
      }
    }
    if (conC3) {
      EventOutputQueue::FireKey::fire_downup(Flags(0), KeyCode::VK_JIS_TEMPORARY_RESTORE, params.keyboardType);
      omit_initialize_ = true;
    }
    return;
  }

  void
  VirtualKey::VK_JIS_IM_CHANGE::reverse_sign_CHANGE_SKIP(int when00)
  {
    if (when00 == 0) {
      pre_counter_plus_minus2_ = counter_plus_minus2_;
    } else if (when00 == 1) {
      if (counter_plus_minus2_ == pre_counter_plus_minus2_ && counter_plus_minus2_ > 0) {
        pre_counter_plus_minus2_ = 0;
        counter_plus_minus2_ = 0;
        if (sign_plus_minus2_ == -99) {
          sign_plus_minus2_ = 1;
        }
        sign_plus_minus2_ = -sign_plus_minus2_;
      } else {}
    } else {
      ++counter_plus_minus2_;
    }
  }

  VirtualKey::VK_JIS_IM_CHANGE::SavedInputModeIndex::Value
  VirtualKey::VK_JIS_IM_CHANGE::IMD2index(InputModeDetail inputmodedetail)
  {
    // inputmodedetail may be two values when input source is Hiragana.
    // - InputModeDetail::JAPANESE          using Kotoeri.
    // - InputModeDetail::JAPANESE_HIRAGANA using AquaSKK.
    //
    // Normalize InputModeDetail::JAPANESE to InputModeDetail::JAPANESE_HIRAGANA.
    if (inputmodedetail == InputModeDetail::JAPANESE) {
      inputmodedetail == InputModeDetail::JAPANESE_HIRAGANA;
    }

    if (inputmodedetail == InputModeDetail::ROMAN)                    { return SavedInputModeIndex::EISU; }
    if (inputmodedetail == InputModeDetail::JAPANESE_HIRAGANA)        { return SavedInputModeIndex::HIRA; }
    if (inputmodedetail == InputModeDetail::JAPANESE_KATAKANA)        { return SavedInputModeIndex::KATA; }
    if (inputmodedetail == InputModeDetail::JAPANESE_HALFWIDTH_KANA)  { return SavedInputModeIndex::HKAT; }
    if (inputmodedetail == InputModeDetail::AINU)                     { return SavedInputModeIndex::AINU; }
    if (inputmodedetail == InputModeDetail::JAPANESE_FULLWIDTH_ROMAN) { return SavedInputModeIndex::FEIS; }

    return SavedInputModeIndex::NONE;
  }

  VirtualKey::VK_JIS_IM_CHANGE::SavedInputModeIndex::Value
  VirtualKey::VK_JIS_IM_CHANGE::modeKey2index(KeyCode key, Flags flags)
  {
    bool CtlSft = (flags == (ModifierFlag::CONTROL_L | ModifierFlag::SHIFT_L) ||
                   flags == (ModifierFlag::CONTROL_L | ModifierFlag::SHIFT_R) ||
                   flags == (ModifierFlag::CONTROL_R | ModifierFlag::SHIFT_L) ||
                   flags == (ModifierFlag::CONTROL_R | ModifierFlag::SHIFT_R) );

    if (key == KeyCode::JIS_EISUU && ! (flags == ModifierFlag::SHIFT_L || flags == ModifierFlag::SHIFT_R) ||
        key == KeyCode::JIS_COLON && CtlSft) {
      return IMD2index(InputModeDetail::ROMAN);
    }

    if (key == KeyCode::JIS_KANA  && ! (flags == ModifierFlag::SHIFT_L || flags == ModifierFlag::SHIFT_R) ||
        key == KeyCode::J && CtlSft) {
      return IMD2index(InputModeDetail::JAPANESE_HIRAGANA);
    }

    if (key == KeyCode::JIS_KANA  && (flags == ModifierFlag::SHIFT_L || flags == ModifierFlag::SHIFT_R) ||
        key == KeyCode::K && CtlSft) {
      return IMD2index(InputModeDetail::JAPANESE_KATAKANA);
    }

    if (key == KeyCode::JIS_EISUU && (flags == ModifierFlag::SHIFT_L || flags == ModifierFlag::SHIFT_R) ||
        key == KeyCode::SEMICOLON && CtlSft) {
      return IMD2index(InputModeDetail::JAPANESE_HALFWIDTH_KANA);
    }

    if (key == KeyCode::JIS_KANA  && (flags == ModifierFlag::OPTION_L || flags == ModifierFlag::OPTION_R) ||
        key == KeyCode::JIS_BRACKET_RIGHT && CtlSft) {
      return IMD2index(InputModeDetail::AINU);
    }

    if (key == KeyCode::L && CtlSft) {
      return IMD2index(InputModeDetail::JAPANESE_FULLWIDTH_ROMAN);
    }

    return SavedInputModeIndex::NONE;
  }

  // XXX: clean up this function.
  bool
  VirtualKey::VK_JIS_IM_CHANGE::control_WSD(ControlWorkspaceDataType type, KeyCode modekey00, Flags flag00, InputModeDetail IMD00)
  {
    int ignore_improveIM = Config::get_essential_config(BRIDGE_ESSENTIAL_CONFIG_INDEX_remap_jis_ignore_improvement_IM_changing);
    SavedInputModeIndex::Value index = SavedInputModeIndex::NONE;
    BridgeWorkSpaceData curWSD00 = CommonData::getcurrent_workspacedata();

    if (type == CONTROL_WORKSPACEDATA_UPDATE) {
      wsd_public_ = curWSD00;

      index = IMD2index(wsd_public_.inputmodedetail);
      set_new_index(index);
      return true;
    }

    if (! (! ignore_improveIM &&
           (type == CONTROL_WORKSPACEDATA_LEARN ||
            type == CONTROL_WORKSPACEDATA_REPLACE ||
            type == CONTROL_WORKSPACEDATA_RESTORE))) {
      return false;
    }

    if (type == CONTROL_WORKSPACEDATA_LEARN) {
      index = IMD2index(curWSD00.inputmodedetail);
    } else if (type == CONTROL_WORKSPACEDATA_REPLACE) {
      index = modeKey2index(modekey00, flag00);
    } else {
      index = IMD2index(IMD00);
    }
    if (index == SavedInputModeIndex::NONE) return false;

    if (type == CONTROL_WORKSPACEDATA_LEARN) {
      if (InputModeDetail::NONE == wsd_learned_.inputmodedetail) {
        wsd_learned_ = curWSD00;
        omit_initialize_ = false;
        set_indexes_directly(SavedInputModeIndex::NONE, index, SavedInputModeIndex::NONE);
      }
      wsd_save_[index] = curWSD00;
      return true;

    } else if ((type == CONTROL_WORKSPACEDATA_REPLACE || type == CONTROL_WORKSPACEDATA_RESTORE) && ! ignore_improveIM) {
      if (InputModeDetail::NONE != wsd_save_[index].inputmodedetail) {
        wsd_public_ = wsd_save_[index];
        if (type == CONTROL_WORKSPACEDATA_REPLACE) {
          set_new_index(index);
        }
      } else {
        return false;
      }
    } else {
      return false;
    }
    return true;
  }

  VirtualKey::VK_JIS_IM_CHANGE::SavedInputModeIndex::Value
  VirtualKey::VK_JIS_IM_CHANGE::get_index_for_seesaw_AtoB_WSD(SeesawType::Value type)
  {
    SavedInputModeIndex::Value tmp_index;
    SavedInputModeIndex::Value Aindex00, Bindex00;

    switch (type) {
      case SeesawType::CUR_PRE:
        Aindex00 = cur_index2_;
        Bindex00 = pre_index2_;
        break;

      case SeesawType::EISUU_KANA:
        Aindex00 = SavedInputModeIndex::EISU;
        Bindex00 = SavedInputModeIndex::HIRA;
        break;

      case SeesawType::KANA_EISUU:
        Aindex00 = SavedInputModeIndex::HIRA;
        Bindex00 = SavedInputModeIndex::EISU;
        break;

      case SeesawType::KANA_OTHERS:
        if (others_index2_ == SavedInputModeIndex::NONE) {
          set_indexes_directly(SavedInputModeIndex::NONE, SavedInputModeIndex::NONE, SavedInputModeIndex::KATA);
        }
        Aindex00 = SavedInputModeIndex::HIRA;
        Bindex00 = others_index2_;
        break;

      case SeesawType::EISUU_OTHERS:
        if (others_index2_ == SavedInputModeIndex::NONE) {
          set_indexes_directly(SavedInputModeIndex::NONE, SavedInputModeIndex::NONE, SavedInputModeIndex::KATA);
        }
        Aindex00 = SavedInputModeIndex::EISU;
        Bindex00 = others_index2_;
        break;

      case SeesawType::NONE:
        return SavedInputModeIndex::NONE;
    }

    if (cur_index2_ != SavedInputModeIndex::NONE && pre_index2_ != SavedInputModeIndex::NONE) {
      if (type == SeesawType::CUR_PRE) {
        set_indexes_directly(Aindex00, Bindex00, SavedInputModeIndex::NONE);
        return cur_index2_;
      } else {
        tmp_index = cur_index2_;
      }

    } else {
      if (type == SeesawType::CUR_PRE) {
        if (pre_index2_ + 1 > SavedInputModeIndex::MAX) {
          set_indexes_directly(SavedInputModeIndex::NONE, SavedInputModeIndex::EISU, SavedInputModeIndex::NONE);
        } else {
          set_indexes_directly(SavedInputModeIndex::NONE, static_cast<SavedInputModeIndex::Value>(pre_index2_ + 1), SavedInputModeIndex::NONE);
        }
        return cur_index2_;
      } else {
        tmp_index = Bindex00;
      }
    }
    if (cur_index2_ != Aindex00) {
      set_indexes_directly(tmp_index, Aindex00, SavedInputModeIndex::NONE);
    } else {
      set_indexes_directly(Aindex00, Bindex00, SavedInputModeIndex::NONE);
    }
    if (seesaw_init2_) {
      if (cur_index2_ != Aindex00) {
        set_indexes_directly(cur_index2_, Aindex00, SavedInputModeIndex::NONE);
      }
      seesaw_init2_ = false;
    }
    return cur_index2_;
  }

  VirtualKey::VK_JIS_IM_CHANGE::SavedInputModeIndex::Value
  VirtualKey::VK_JIS_IM_CHANGE::get_index_for_replaceWSD(int sign00, int skip[], ReplaceType::Value replacetype)
  {
    SavedInputModeIndex::Value ret;
    SavedInputModeIndex::Value cur_index_tmp, others_index_tmp;

    cur_index_tmp    = cur_index2_;
    others_index_tmp = others_index2_;

    bool cond00 = (cur_index2_ == SavedInputModeIndex::EISU);
    bool cond01 = (pre_index2_ == SavedInputModeIndex::HKAT && cur_index2_ == SavedInputModeIndex::KATA);
    bool cond02 = (pre_index2_ != SavedInputModeIndex::KATA && cur_index2_ == SavedInputModeIndex::HKAT);
    bool cond10 = (pre_index2_ == SavedInputModeIndex::EISU && cur_index2_ == SavedInputModeIndex::HIRA);
    bool cond11 = (pre_index2_ == SavedInputModeIndex::KATA && cur_index2_ == SavedInputModeIndex::HIRA && others_index2_ == SavedInputModeIndex::KATA);
    bool cond12 = (pre_index2_ == SavedInputModeIndex::HKAT && cur_index2_ == SavedInputModeIndex::HIRA && others_index2_ == SavedInputModeIndex::HKAT);
    bool cond13 = (pre_index2_ != SavedInputModeIndex::HKAT && cur_index2_ == SavedInputModeIndex::KATA);
    bool cond14 = (pre_index2_ == SavedInputModeIndex::KATA && cur_index2_ == SavedInputModeIndex::HKAT);
    if (replacetype == ReplaceType::SKIP_PREVIOUS) {
      skip[pre_index2_] = 1;

    } else if (replacetype == ReplaceType::SKIP_SPECIFIC) {
      if (sign_plus_minus2_ == -99) {
        sign_plus_minus2_ = 1;
      }
      if (sign_plus_minus2_ == 1 &&
          (cond00 || cond01 || cond02)) {
        sign_plus_minus2_ = -1;
        if (cond00) {
          others_index_tmp = SavedInputModeIndex::EISU;
        }
      } else if (sign_plus_minus2_ == -1     && (
                   cond10 || cond11 || cond12 || cond13 || cond14)) {
        sign_plus_minus2_ = 1;
      } else {}
      sign00 = sign_plus_minus2_;
    }

    int continue_end00 = 0;
    for (int i = (cur_index_tmp == SavedInputModeIndex::NONE ? 1 : cur_index_tmp);;) {
      if (i > SavedInputModeIndex::MAX && sign00 == 1 || i < 1 && sign00 == -1) {
        if (continue_end00 == 1) {
          ret = SavedInputModeIndex::NONE;
          break;
        }
        if (sign00 == 1) {
          i = 1;
        } else {
          i = SavedInputModeIndex::MAX;
        }
        continue_end00 = 1;
        continue;
      }

      if (cur_index_tmp != SavedInputModeIndex::NONE && cur_index_tmp != i &&
          others_index_tmp != i) {
        if (skip[i] != 1) {
          ret = static_cast<SavedInputModeIndex::Value>(i);
          break;
        }
      }
      i = i + sign00;
    }

    if (ret != SavedInputModeIndex::NONE) {
      if (replacetype == ReplaceType::SKIP_PREVIOUS) {
        set_indexes_directly(SavedInputModeIndex::NONE, ret, SavedInputModeIndex::NONE);
      } else {
        set_indexes_directly(cur_index_tmp, ret, SavedInputModeIndex::NONE);
      }
      if (replacetype == ReplaceType::SKIP_PREVIOUS || replacetype == ReplaceType::SKIP_SPECIFIC) {
        set_indexes_directly(SavedInputModeIndex::NONE, SavedInputModeIndex::NONE, ret);
      }
    } else {
      ret = cur_index_tmp;
    }

    return ret;
  }

  void
  VirtualKey::VK_JIS_IM_CHANGE::set_indexes_directly(SavedInputModeIndex::Value new_pre, SavedInputModeIndex::Value new_cur, SavedInputModeIndex::Value new_others)
  {
    if (new_pre != SavedInputModeIndex::NONE) {
      pre_index2_  = new_pre;
    }
    if (new_cur != SavedInputModeIndex::NONE) {
      cur_index2_ = new_cur;
    }
    if (new_others != SavedInputModeIndex::NONE) {
      others_index2_ = new_others;
    }
  }

  void
  VirtualKey::VK_JIS_IM_CHANGE::set_new_index(SavedInputModeIndex::Value index)
  {
    if (cur_index2_ != SavedInputModeIndex::NONE && pre_index2_ != SavedInputModeIndex::NONE) {
      if (cur_index2_ != index) {
        set_indexes_directly(cur_index2_, index, SavedInputModeIndex::NONE);
      }
    } else if (cur_index2_ == SavedInputModeIndex::NONE) {
      if (pre_index2_ != index) {
        set_indexes_directly(SavedInputModeIndex::NONE, index, SavedInputModeIndex::NONE);
      }
    } else {
      set_indexes_directly(index, SavedInputModeIndex::NONE, SavedInputModeIndex::NONE);
    }
  }
}
