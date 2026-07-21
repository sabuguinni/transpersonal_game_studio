#include "Narr_CharacterVoices.h"
#include "Engine/Engine.h"

UNarr_CharacterVoices::UNarr_CharacterVoices()
{
    InitializeDefaultVoices();
}

FNarr_VoiceEntry UNarr_CharacterVoices::GetVoiceByType(ENarr_VoiceType VoiceType) const
{
    for (const FNarr_VoiceEntry& Voice : VoiceEntries)
    {
        if (Voice.VoiceType == VoiceType)
        {
            return Voice;
        }
    }
    
    // Return default entry if not found
    return FNarr_VoiceEntry();
}

FNarr_VoiceEntry UNarr_CharacterVoices::GetVoiceByName(const FString& CharacterName) const
{
    for (const FNarr_VoiceEntry& Voice : VoiceEntries)
    {
        if (Voice.CharacterName == CharacterName)
        {
            return Voice;
        }
    }
    
    return FNarr_VoiceEntry();
}

void UNarr_CharacterVoices::AddVoiceEntry(const FNarr_VoiceEntry& NewVoice)
{
    VoiceEntries.Add(NewVoice);
    UE_LOG(LogTemp, Log, TEXT("Voice System: Added voice for %s (%s)"), 
        *NewVoice.CharacterName, *NewVoice.VoiceDescription);
}

TArray<FNarr_VoiceEntry> UNarr_CharacterVoices::GetAllVoicesOfType(ENarr_VoiceType VoiceType) const
{
    TArray<FNarr_VoiceEntry> FilteredVoices;
    
    for (const FNarr_VoiceEntry& Voice : VoiceEntries)
    {
        if (Voice.VoiceType == VoiceType)
        {
            FilteredVoices.Add(Voice);
        }
    }
    
    return FilteredVoices;
}

void UNarr_CharacterVoices::InitializeDefaultVoices()
{
    VoiceEntries.Empty();

    // Survival Narrator Voice
    FNarr_VoiceEntry NarratorVoice;
    NarratorVoice.VoiceType = ENarr_VoiceType::Narrator;
    NarratorVoice.CharacterName = TEXT("Survival Narrator");
    NarratorVoice.VoiceDescription = TEXT("Deep, authoritative voice for survival guidance");
    NarratorVoice.SampleDialogue = FText::FromString(TEXT("The great hunters move through these ancient lands. Stone tools and fire are your only allies against the massive predators that rule this world."));
    NarratorVoice.BasePitch = 0.9f;
    NarratorVoice.BaseVolume = 1.0f;
    VoiceEntries.Add(NarratorVoice);

    // Tribal Warrior Voice
    FNarr_VoiceEntry WarriorVoice;
    WarriorVoice.VoiceType = ENarr_VoiceType::Warrior;
    WarriorVoice.CharacterName = TEXT("Tribal Warrior");
    WarriorVoice.VoiceDescription = TEXT("Urgent, battle-hardened voice for danger warnings");
    WarriorVoice.SampleDialogue = FText::FromString(TEXT("Danger approaches! The pack hunters have caught your scent. Find high ground or prepare to fight with everything you have."));
    WarriorVoice.BasePitch = 1.1f;
    WarriorVoice.BaseVolume = 1.2f;
    VoiceEntries.Add(WarriorVoice);

    // Tribal Elder Voice
    FNarr_VoiceEntry ElderVoice;
    ElderVoice.VoiceType = ENarr_VoiceType::TribalElder;
    ElderVoice.CharacterName = TEXT("Tribal Elder");
    ElderVoice.VoiceDescription = TEXT("Wise, weathered voice sharing ancient knowledge");
    ElderVoice.SampleDialogue = FText::FromString(TEXT("Welcome to the ancient hunting grounds. Survival depends on your wits and courage."));
    ElderVoice.BasePitch = 0.8f;
    ElderVoice.BaseVolume = 0.9f;
    VoiceEntries.Add(ElderVoice);

    // Scout Voice
    FNarr_VoiceEntry ScoutVoice;
    ScoutVoice.VoiceType = ENarr_VoiceType::Scout;
    ScoutVoice.CharacterName = TEXT("Tribal Scout");
    ScoutVoice.VoiceDescription = TEXT("Alert, quick voice for warnings and discoveries");
    ScoutVoice.SampleDialogue = FText::FromString(TEXT("Beware the great predators! They hunt in packs and show no mercy."));
    ScoutVoice.BasePitch = 1.2f;
    ScoutVoice.BaseVolume = 1.1f;
    VoiceEntries.Add(ScoutVoice);

    // Craftsman Voice
    FNarr_VoiceEntry CraftsmanVoice;
    CraftsmanVoice.VoiceType = ENarr_VoiceType::Craftsman;
    CraftsmanVoice.CharacterName = TEXT("Tribal Craftsman");
    CraftsmanVoice.VoiceDescription = TEXT("Practical, knowledgeable voice for survival tips");
    CraftsmanVoice.SampleDialogue = FText::FromString(TEXT("Stone and wood are your allies. Craft tools to increase your chances of survival."));
    CraftsmanVoice.BasePitch = 1.0f;
    CraftsmanVoice.BaseVolume = 0.95f;
    VoiceEntries.Add(CraftsmanVoice);

    // Beast Voice (for dinosaur sounds/growls)
    FNarr_VoiceEntry BeastVoice;
    BeastVoice.VoiceType = ENarr_VoiceType::Beast;
    BeastVoice.CharacterName = TEXT("Prehistoric Beast");
    BeastVoice.VoiceDescription = TEXT("Deep growls and roars for dinosaur vocalizations");
    BeastVoice.SampleDialogue = FText::FromString(TEXT("*Deep territorial roar echoing through the valley*"));
    BeastVoice.BasePitch = 0.6f;
    BeastVoice.BaseVolume = 1.3f;
    VoiceEntries.Add(BeastVoice);

    UE_LOG(LogTemp, Log, TEXT("Voice System: Initialized %d default voice entries"), VoiceEntries.Num());
}