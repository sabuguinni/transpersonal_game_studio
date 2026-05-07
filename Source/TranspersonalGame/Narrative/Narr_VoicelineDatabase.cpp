#include "Narr_VoicelineDatabase.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"

UNarr_VoicelineDatabase::UNarr_VoicelineDatabase()
{
    // Initialize default voiceline categories
    InitializeDefaultVoicelines();
}

void UNarr_VoicelineDatabase::InitializeDefaultVoicelines()
{
    // Clear existing data
    VoicelineCategories.Empty();
    
    // SURVIVAL OBSERVATIONS
    FNarr_VoicelineCategory SurvivalCategory;
    SurvivalCategory.CategoryName = "Survival";
    SurvivalCategory.Description = "Field observations and survival commentary";
    
    // Field researcher voicelines
    FNarr_VoicelineEntry FieldEntry1;
    FieldEntry1.VoicelineID = "FIELD_RAPTOR_PACK";
    FieldEntry1.CharacterName = "Field Researcher";
    FieldEntry1.DialogueText = "Day 47 in the wilderness. The pack of Velociraptors has been tracking me for three days now. They're intelligent, coordinated hunters.";
    FieldEntry1.AudioURL = "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778122289175_FieldResearcher.mp3";
    FieldEntry1.Duration = 28.0f;
    FieldEntry1.TriggerConditions.Add("NearRaptorPack");
    FieldEntry1.EmotionalTone = ENarr_EmotionalTone::Tense;
    FieldEntry1.Priority = ENarr_VoicelinePriority::High;
    SurvivalCategory.Voicelines.Add(FieldEntry1);
    
    VoicelineCategories.Add("Survival", SurvivalCategory);
    
    // SAFETY ALERTS
    FNarr_VoicelineCategory SafetyCategory;
    SafetyCategory.CategoryName = "Safety";
    SafetyCategory.Description = "Emergency alerts and safety warnings";
    
    // Brachiosaurus alert
    FNarr_VoicelineEntry SafetyEntry1;
    SafetyEntry1.VoicelineID = "SAFETY_BRACHIO_ALERT";
    SafetyEntry1.CharacterName = "Safety Coordinator";
    SafetyEntry1.DialogueText = "Alert! Massive footsteps detected approaching from the north. Seismic readings indicate a Brachiosaurus herd moving through the forest canopy.";
    SafetyEntry1.AudioURL = "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778122297329_SafetyCoordinator.mp3";
    SafetyEntry1.Duration = 25.0f;
    SafetyEntry1.TriggerConditions.Add("BrachiosaurusNearby");
    SafetyEntry1.EmotionalTone = ENarr_EmotionalTone::Alert;
    SafetyEntry1.Priority = ENarr_VoicelinePriority::Critical;
    SafetyCategory.Voicelines.Add(SafetyEntry1);
    
    // T-Rex warning
    FNarr_VoicelineEntry SafetyEntry2;
    SafetyEntry2.VoicelineID = "SAFETY_TREX_WARNING";
    SafetyEntry2.CharacterName = "Emergency Dispatcher";
    SafetyEntry2.DialogueText = "Warning! Tyrannosaurus Rex detected in sector 7. This apex predator is over 12 meters long and weighs approximately 8 tons.";
    SafetyEntry2.AudioURL = "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778122315533_EmergencyDispatcher.mp3";
    SafetyEntry2.Duration = 24.0f;
    SafetyEntry2.TriggerConditions.Add("TRexDetected");
    SafetyEntry2.EmotionalTone = ENarr_EmotionalTone::Urgent;
    SafetyEntry2.Priority = ENarr_VoicelinePriority::Critical;
    SafetyCategory.Voicelines.Add(SafetyEntry2);
    
    VoicelineCategories.Add("Safety", SafetyCategory);
    
    // EDUCATIONAL CONTENT
    FNarr_VoicelineCategory EducationCategory;
    EducationCategory.CategoryName = "Education";
    EducationCategory.Description = "Educational content about dinosaur behavior";
    
    // Triceratops behavior
    FNarr_VoicelineEntry EduEntry1;
    EduEntry1.VoicelineID = "EDU_TRICERATOPS_BEHAVIOR";
    EduEntry1.CharacterName = "Naturalist Guide";
    EduEntry1.DialogueText = "The Triceratops herd has established a defensive formation around the water source. Three adults form a protective triangle while the juveniles drink safely.";
    EduEntry1.AudioURL = "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778122307036_NaturalistGuide.mp3";
    EduEntry1.Duration = 24.0f;
    EduEntry1.TriggerConditions.Add("TriceratopsHerd");
    EduEntry1.EmotionalTone = ENarr_EmotionalTone::Informative;
    EduEntry1.Priority = ENarr_VoicelinePriority::Medium;
    EducationCategory.Voicelines.Add(EduEntry1);
    
    VoicelineCategories.Add("Education", EducationCategory);
    
    UE_LOG(LogTemp, Log, TEXT("VoicelineDatabase: Initialized with %d categories"), VoicelineCategories.Num());
}

FNarr_VoicelineEntry* UNarr_VoicelineDatabase::GetVoicelineByID(const FString& VoicelineID)
{
    for (auto& CategoryPair : VoicelineCategories)
    {
        for (FNarr_VoicelineEntry& Entry : CategoryPair.Value.Voicelines)
        {
            if (Entry.VoicelineID == VoicelineID)
            {
                return &Entry;
            }
        }
    }
    return nullptr;
}

TArray<FNarr_VoicelineEntry*> UNarr_VoicelineDatabase::GetVoicelinesByCategory(const FString& CategoryName)
{
    TArray<FNarr_VoicelineEntry*> Result;
    
    if (FNarr_VoicelineCategory* Category = VoicelineCategories.Find(CategoryName))
    {
        for (FNarr_VoicelineEntry& Entry : Category->Voicelines)
        {
            Result.Add(&Entry);
        }
    }
    
    return Result;
}

TArray<FNarr_VoicelineEntry*> UNarr_VoicelineDatabase::GetVoicelinesByTrigger(const FString& TriggerCondition)
{
    TArray<FNarr_VoicelineEntry*> Result;
    
    for (auto& CategoryPair : VoicelineCategories)
    {
        for (FNarr_VoicelineEntry& Entry : CategoryPair.Value.Voicelines)
        {
            if (Entry.TriggerConditions.Contains(TriggerCondition))
            {
                Result.Add(&Entry);
            }
        }
    }
    
    return Result;
}

TArray<FNarr_VoicelineEntry*> UNarr_VoicelineDatabase::GetVoicelinesByPriority(ENarr_VoicelinePriority Priority)
{
    TArray<FNarr_VoicelineEntry*> Result;
    
    for (auto& CategoryPair : VoicelineCategories)
    {
        for (FNarr_VoicelineEntry& Entry : CategoryPair.Value.Voicelines)
        {
            if (Entry.Priority == Priority)
            {
                Result.Add(&Entry);
            }
        }
    }
    
    return Result;
}

bool UNarr_VoicelineDatabase::AddVoiceline(const FString& CategoryName, const FNarr_VoicelineEntry& NewVoiceline)
{
    if (!VoicelineCategories.Contains(CategoryName))
    {
        // Create new category if it doesn't exist
        FNarr_VoicelineCategory NewCategory;
        NewCategory.CategoryName = CategoryName;
        NewCategory.Description = FString::Printf(TEXT("Auto-created category: %s"), *CategoryName);
        VoicelineCategories.Add(CategoryName, NewCategory);
    }
    
    // Check for duplicate IDs
    if (GetVoicelineByID(NewVoiceline.VoicelineID) != nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("VoicelineDatabase: Duplicate ID %s"), *NewVoiceline.VoicelineID);
        return false;
    }
    
    VoicelineCategories[CategoryName].Voicelines.Add(NewVoiceline);
    UE_LOG(LogTemp, Log, TEXT("VoicelineDatabase: Added voiceline %s to category %s"), 
           *NewVoiceline.VoicelineID, *CategoryName);
    
    return true;
}

bool UNarr_VoicelineDatabase::RemoveVoiceline(const FString& VoicelineID)
{
    for (auto& CategoryPair : VoicelineCategories)
    {
        for (int32 i = CategoryPair.Value.Voicelines.Num() - 1; i >= 0; i--)
        {
            if (CategoryPair.Value.Voicelines[i].VoicelineID == VoicelineID)
            {
                CategoryPair.Value.Voicelines.RemoveAt(i);
                UE_LOG(LogTemp, Log, TEXT("VoicelineDatabase: Removed voiceline %s"), *VoicelineID);
                return true;
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("VoicelineDatabase: Voiceline %s not found"), *VoicelineID);
    return false;
}

void UNarr_VoicelineDatabase::ClearAllVoicelines()
{
    VoicelineCategories.Empty();
    UE_LOG(LogTemp, Log, TEXT("VoicelineDatabase: Cleared all voicelines"));
}

int32 UNarr_VoicelineDatabase::GetTotalVoicelineCount() const
{
    int32 Total = 0;
    for (const auto& CategoryPair : VoicelineCategories)
    {
        Total += CategoryPair.Value.Voicelines.Num();
    }
    return Total;
}

TArray<FString> UNarr_VoicelineDatabase::GetAllCategoryNames() const
{
    TArray<FString> CategoryNames;
    VoicelineCategories.GetKeys(CategoryNames);
    return CategoryNames;
}

FString UNarr_VoicelineDatabase::GetDatabaseStats() const
{
    FString Stats = FString::Printf(TEXT("Voiceline Database Stats:\n"));
    Stats += FString::Printf(TEXT("Total Categories: %d\n"), VoicelineCategories.Num());
    Stats += FString::Printf(TEXT("Total Voicelines: %d\n\n"), GetTotalVoicelineCount());
    
    for (const auto& CategoryPair : VoicelineCategories)
    {
        Stats += FString::Printf(TEXT("Category '%s': %d voicelines\n"), 
                                *CategoryPair.Key, 
                                CategoryPair.Value.Voicelines.Num());
    }
    
    return Stats;
}