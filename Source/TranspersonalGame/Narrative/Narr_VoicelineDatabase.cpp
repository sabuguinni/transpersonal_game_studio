#include "Narr_VoicelineDatabase.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UNarr_VoicelineDatabase::UNarr_VoicelineDatabase()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize with survival-focused voicelines
    InitializeDefaultVoicelines();
}

void UNarr_VoicelineDatabase::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache frequently used voicelines for performance
    CacheFrequentVoicelines();
}

void UNarr_VoicelineDatabase::InitializeDefaultVoicelines()
{
    VoicelineDatabase.Empty();
    
    // Survival discovery voicelines
    FNarr_VoicelineEntry DiscoveryEntry;
    DiscoveryEntry.VoicelineID = "DISCOVERY_CAVE_PAINTINGS";
    DiscoveryEntry.AudioURL = "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778153176470_DiscoveryNarrator.mp3";
    DiscoveryEntry.TranscriptText = "Discovery log, day 115. Found ancient cave paintings in the limestone caverns near the waterfall. The markings suggest previous human inhabitants understood dinosaur behavior patterns.";
    DiscoveryEntry.Duration = 28.0f;
    DiscoveryEntry.Category = ENarr_VoicelineCategory::Discovery;
    DiscoveryEntry.Priority = ENarr_VoicelinePriority::Medium;
    DiscoveryEntry.TriggerConditions.Add("PlayerNearCavePaintings");
    DiscoveryEntry.TriggerConditions.Add("FirstTimeDiscovery");
    VoicelineDatabase.Add(DiscoveryEntry.VoicelineID, DiscoveryEntry);
    
    // Emergency alert voicelines
    FNarr_VoicelineEntry EmergencyEntry;
    EmergencyEntry.VoicelineID = "EMERGENCY_TRICERATOPS_HERD";
    EmergencyEntry.AudioURL = "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778153167141_EmergencyBroadcast.mp3";
    EmergencyEntry.TranscriptText = "Critical alert! Massive Triceratops herd detected approaching from the northwest. Count exceeds fifty individuals, including juveniles.";
    EmergencyEntry.Duration = 27.0f;
    EmergencyEntry.Category = ENarr_VoicelineCategory::Emergency;
    EmergencyEntry.Priority = ENarr_VoicelinePriority::Critical;
    EmergencyEntry.TriggerConditions.Add("LargeHerbivoreHerdDetected");
    EmergencyEntry.TriggerConditions.Add("PlayerInDangerZone");
    VoicelineDatabase.Add(EmergencyEntry.VoicelineID, EmergencyEntry);
    
    // Tactical advice voicelines
    FNarr_VoicelineEntry TacticalEntry;
    TacticalEntry.VoicelineID = "TACTICAL_ALLOSAURUS_WARNING";
    TacticalEntry.AudioURL = "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778153187235_TacticalAdvisor.mp3";
    TacticalEntry.TranscriptText = "Danger assessment update. The Allosaurus pack has established new territorial boundaries extending into the southern marsh region.";
    TacticalEntry.Duration = 30.0f;
    TacticalEntry.Category = ENarr_VoicelineCategory::Tactical;
    TacticalEntry.Priority = ENarr_VoicelinePriority::High;
    TacticalEntry.TriggerConditions.Add("PredatorTerritoryDetected");
    TacticalEntry.TriggerConditions.Add("PlayerApproachingDanger");
    VoicelineDatabase.Add(TacticalEntry.VoicelineID, TacticalEntry);
    
    // Survival narration voicelines
    FNarr_VoicelineEntry SurvivalEntry;
    SurvivalEntry.VoicelineID = "SURVIVAL_PACK_DYNAMICS";
    SurvivalEntry.AudioURL = "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778153158274_SurvivalNarrator.mp3";
    SurvivalEntry.TranscriptText = "Day 112 in the wilderness. The pack dynamics have shifted dramatically since the volcanic eruption. The alpha Raptor has been challenged by a younger male.";
    SurvivalEntry.Duration = 29.0f;
    SurvivalEntry.Category = ENarr_VoicelineCategory::Narration;
    SurvivalEntry.Priority = ENarr_VoicelinePriority::Medium;
    SurvivalEntry.TriggerConditions.Add("DailyNarrationTime");
    SurvivalEntry.TriggerConditions.Add("PackBehaviorObserved");
    VoicelineDatabase.Add(SurvivalEntry.VoicelineID, SurvivalEntry);
}

void UNarr_VoicelineDatabase::CacheFrequentVoicelines()
{
    CachedVoicelines.Empty();
    
    // Cache high-priority and frequently triggered voicelines
    for (const auto& VoicelinePair : VoicelineDatabase)
    {
        const FNarr_VoicelineEntry& Entry = VoicelinePair.Value;
        if (Entry.Priority == ENarr_VoicelinePriority::Critical || 
            Entry.Priority == ENarr_VoicelinePriority::High)
        {
            CachedVoicelines.Add(Entry.VoicelineID, Entry);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Cached %d high-priority voicelines"), CachedVoicelines.Num());
}

FNarr_VoicelineEntry UNarr_VoicelineDatabase::GetVoicelineByID(const FString& VoicelineID) const
{
    // Check cache first for performance
    if (CachedVoicelines.Contains(VoicelineID))
    {
        return CachedVoicelines[VoicelineID];
    }
    
    // Fall back to main database
    if (VoicelineDatabase.Contains(VoicelineID))
    {
        return VoicelineDatabase[VoicelineID];
    }
    
    // Return empty entry if not found
    return FNarr_VoicelineEntry();
}

TArray<FNarr_VoicelineEntry> UNarr_VoicelineDatabase::GetVoicelinesByCategory(ENarr_VoicelineCategory Category) const
{
    TArray<FNarr_VoicelineEntry> CategoryVoicelines;
    
    for (const auto& VoicelinePair : VoicelineDatabase)
    {
        if (VoicelinePair.Value.Category == Category)
        {
            CategoryVoicelines.Add(VoicelinePair.Value);
        }
    }
    
    // Sort by priority (Critical first, then High, Medium, Low)
    CategoryVoicelines.Sort([](const FNarr_VoicelineEntry& A, const FNarr_VoicelineEntry& B)
    {
        return static_cast<int32>(A.Priority) > static_cast<int32>(B.Priority);
    });
    
    return CategoryVoicelines;
}

TArray<FNarr_VoicelineEntry> UNarr_VoicelineDatabase::GetVoicelinesByPriority(ENarr_VoicelinePriority Priority) const
{
    TArray<FNarr_VoicelineEntry> PriorityVoicelines;
    
    for (const auto& VoicelinePair : VoicelineDatabase)
    {
        if (VoicelinePair.Value.Priority == Priority)
        {
            PriorityVoicelines.Add(VoicelinePair.Value);
        }
    }
    
    return PriorityVoicelines;
}

bool UNarr_VoicelineDatabase::AddVoiceline(const FNarr_VoicelineEntry& NewVoiceline)
{
    if (NewVoiceline.VoicelineID.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot add voiceline with empty ID"));
        return false;
    }
    
    if (VoicelineDatabase.Contains(NewVoiceline.VoicelineID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Voiceline ID already exists: %s"), *NewVoiceline.VoicelineID);
        return false;
    }
    
    VoicelineDatabase.Add(NewVoiceline.VoicelineID, NewVoiceline);
    
    // Add to cache if high priority
    if (NewVoiceline.Priority == ENarr_VoicelinePriority::Critical || 
        NewVoiceline.Priority == ENarr_VoicelinePriority::High)
    {
        CachedVoicelines.Add(NewVoiceline.VoicelineID, NewVoiceline);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Added voiceline: %s"), *NewVoiceline.VoicelineID);
    return true;
}

bool UNarr_VoicelineDatabase::RemoveVoiceline(const FString& VoicelineID)
{
    if (!VoicelineDatabase.Contains(VoicelineID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Voiceline not found for removal: %s"), *VoicelineID);
        return false;
    }
    
    VoicelineDatabase.Remove(VoicelineID);
    CachedVoicelines.Remove(VoicelineID);
    
    UE_LOG(LogTemp, Log, TEXT("Removed voiceline: %s"), *VoicelineID);
    return true;
}

bool UNarr_VoicelineDatabase::UpdateVoiceline(const FString& VoicelineID, const FNarr_VoicelineEntry& UpdatedVoiceline)
{
    if (!VoicelineDatabase.Contains(VoicelineID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Voiceline not found for update: %s"), *VoicelineID);
        return false;
    }
    
    VoicelineDatabase[VoicelineID] = UpdatedVoiceline;
    
    // Update cache if applicable
    if (CachedVoicelines.Contains(VoicelineID))
    {
        if (UpdatedVoiceline.Priority == ENarr_VoicelinePriority::Critical || 
            UpdatedVoiceline.Priority == ENarr_VoicelinePriority::High)
        {
            CachedVoicelines[VoicelineID] = UpdatedVoiceline;
        }
        else
        {
            CachedVoicelines.Remove(VoicelineID);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Updated voiceline: %s"), *VoicelineID);
    return true;
}

TArray<FString> UNarr_VoicelineDatabase::GetAllVoicelineIDs() const
{
    TArray<FString> VoicelineIDs;
    VoicelineDatabase.GetKeys(VoicelineIDs);
    return VoicelineIDs;
}

int32 UNarr_VoicelineDatabase::GetVoicelineCount() const
{
    return VoicelineDatabase.Num();
}

void UNarr_VoicelineDatabase::ClearAllVoicelines()
{
    VoicelineDatabase.Empty();
    CachedVoicelines.Empty();
    UE_LOG(LogTemp, Log, TEXT("Cleared all voicelines from database"));
}

bool UNarr_VoicelineDatabase::IsVoicelineValid(const FString& VoicelineID) const
{
    if (!VoicelineDatabase.Contains(VoicelineID))
    {
        return false;
    }
    
    const FNarr_VoicelineEntry& Entry = VoicelineDatabase[VoicelineID];
    return !Entry.VoicelineID.IsEmpty() && 
           !Entry.AudioURL.IsEmpty() && 
           Entry.Duration > 0.0f;
}

FNarr_VoicelineEntry UNarr_VoicelineDatabase::GetRandomVoicelineByCategory(ENarr_VoicelineCategory Category) const
{
    TArray<FNarr_VoicelineEntry> CategoryVoicelines = GetVoicelinesByCategory(Category);
    
    if (CategoryVoicelines.Num() == 0)
    {
        return FNarr_VoicelineEntry();
    }
    
    int32 RandomIndex = FMath::RandRange(0, CategoryVoicelines.Num() - 1);
    return CategoryVoicelines[RandomIndex];
}