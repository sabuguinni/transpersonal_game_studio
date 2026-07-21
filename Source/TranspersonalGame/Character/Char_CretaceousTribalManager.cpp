#include "Char_CretaceousTribalManager.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

UChar_CretaceousTribalManager::UChar_CretaceousTribalManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 2.0f;
    
    bIsInitialized = false;
    LastSpawnTime = 0.0f;
    
    // Initialize default configuration
    TribalConfig = FChar_CretaceousTribalConfig();
}

void UChar_CretaceousTribalManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeTribalSystem();
}

void UChar_CretaceousTribalManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsInitialized)
    {
        CleanupInvalidNPCs();
        
        // Update last spawn time
        LastSpawnTime += DeltaTime;
    }
}

void UChar_CretaceousTribalManager::InitializeTribalSystem()
{
    if (bIsInitialized)
    {
        return;
    }
    
    ValidateTribalConfiguration();
    
    // Clear any existing NPCs
    ActiveTribalNPCs.Empty();
    
    // Create default tribal variations if none exist
    if (TribalConfig.TribalVariations.Num() == 0)
    {
        TribalConfig.TribalVariations.Add(CreateTribalVariation(EChar_TribalRole::Hunter));
        TribalConfig.TribalVariations.Add(CreateTribalVariation(EChar_TribalRole::Gatherer));
        TribalConfig.TribalVariations.Add(CreateTribalVariation(EChar_TribalRole::Crafter));
        TribalConfig.TribalVariations.Add(CreateTribalVariation(EChar_TribalRole::Elder));
    }
    
    bIsInitialized = true;
    
    UE_LOG(LogTemp, Warning, TEXT("Cretaceous Tribal Manager initialized with %d variations"), TribalConfig.TribalVariations.Num());
}

void UChar_CretaceousTribalManager::SpawnTribalNPC(const FVector& Location, const FChar_TribalAppearanceData& AppearanceData)
{
    if (!bIsInitialized)
    {
        InitializeTribalSystem();
    }
    
    if (ActiveTribalNPCs.Num() >= TribalConfig.MaxTribalNPCs)
    {
        UE_LOG(LogTemp, Warning, TEXT("Maximum tribal NPCs reached (%d)"), TribalConfig.MaxTribalNPCs);
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Try to spawn a basic pawn for now - in a real implementation this would be a custom tribal character class
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
    
    APawn* NewNPC = World->SpawnActor<APawn>(APawn::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
    if (NewNPC)
    {
        NewNPC->SetActorLabel(FString::Printf(TEXT("TribalNPC_%s"), *AppearanceData.CharacterName));
        
        ApplyTribalAppearance(NewNPC, AppearanceData);
        
        ActiveTribalNPCs.Add(NewNPC);
        
        UE_LOG(LogTemp, Log, TEXT("Spawned tribal NPC: %s at location %s"), *AppearanceData.CharacterName, *Location.ToString());
    }
}

void UChar_CretaceousTribalManager::ApplyTribalAppearance(APawn* TargetPawn, const FChar_TribalAppearanceData& AppearanceData)
{
    if (!TargetPawn)
    {
        return;
    }
    
    // Apply tribal markings
    if (TribalConfig.bEnableTribalMarkings)
    {
        UpdateTribalMarkings(TargetPawn, AppearanceData.TribalMarkings);
    }
    
    // Apply weathering effects based on age and role
    if (TribalConfig.bEnableWeatheringEffects)
    {
        float WeatheringIntensity = FMath::Clamp(AppearanceData.Age / 60.0f, 0.1f, 1.0f);
        ApplyWeatheringEffects(TargetPawn, WeatheringIntensity);
    }
    
    // Set clothing and tools
    SetTribalClothing(TargetPawn, AppearanceData.ClothingItems);
    EquipTribalTools(TargetPawn, AppearanceData.ToolsAndWeapons);
    
    UE_LOG(LogTemp, Log, TEXT("Applied tribal appearance to %s (Role: %d, Age: %.1f)"), 
           *TargetPawn->GetName(), (int32)AppearanceData.TribalRole, AppearanceData.Age);
}

FChar_TribalAppearanceData UChar_CretaceousTribalManager::GenerateRandomTribalAppearance()
{
    FChar_TribalAppearanceData NewAppearance;
    
    // Random role
    int32 RoleIndex = FMath::RandRange(0, 3);
    NewAppearance.TribalRole = static_cast<EChar_TribalRole>(RoleIndex);
    
    // Random skin tone
    int32 SkinIndex = FMath::RandRange(0, 4);
    NewAppearance.SkinTone = static_cast<EChar_SkinTone>(SkinIndex);
    
    // Random body type
    int32 BodyIndex = FMath::RandRange(0, 2);
    NewAppearance.BodyType = static_cast<EChar_BodyType>(BodyIndex);
    
    // Random age based on role
    switch (NewAppearance.TribalRole)
    {
        case EChar_TribalRole::Elder:
            NewAppearance.Age = FMath::RandRange(50.0f, 70.0f);
            break;
        case EChar_TribalRole::Hunter:
            NewAppearance.Age = FMath::RandRange(20.0f, 45.0f);
            break;
        case EChar_TribalRole::Gatherer:
            NewAppearance.Age = FMath::RandRange(18.0f, 50.0f);
            break;
        case EChar_TribalRole::Crafter:
            NewAppearance.Age = FMath::RandRange(25.0f, 55.0f);
            break;
        default:
            NewAppearance.Age = FMath::RandRange(20.0f, 40.0f);
            break;
    }
    
    // Generate random name
    TArray<FString> NamePrefixes = {TEXT("Krog"), TEXT("Thak"), TEXT("Uga"), TEXT("Brok"), TEXT("Zara"), TEXT("Nala"), TEXT("Kira")};
    TArray<FString> NameSuffixes = {TEXT("tooth"), TEXT("stone"), TEXT("fire"), TEXT("wind"), TEXT("bone"), TEXT("claw"), TEXT("eye")};
    
    FString RandomPrefix = NamePrefixes[FMath::RandRange(0, NamePrefixes.Num() - 1)];
    FString RandomSuffix = NameSuffixes[FMath::RandRange(0, NameSuffixes.Num() - 1)];
    NewAppearance.CharacterName = FString::Printf(TEXT("%s_%s"), *RandomPrefix, *RandomSuffix);
    
    // Add tribal markings based on role
    switch (NewAppearance.TribalRole)
    {
        case EChar_TribalRole::Hunter:
            NewAppearance.TribalMarkings.Add(TEXT("FaceStripes_Red"));
            NewAppearance.TribalMarkings.Add(TEXT("ArmBands_Black"));
            break;
        case EChar_TribalRole::Elder:
            NewAppearance.TribalMarkings.Add(TEXT("ForeheadSymbol_White"));
            NewAppearance.TribalMarkings.Add(TEXT("ChinDots_Blue"));
            break;
        case EChar_TribalRole::Gatherer:
            NewAppearance.TribalMarkings.Add(TEXT("CheekLines_Green"));
            break;
        case EChar_TribalRole::Crafter:
            NewAppearance.TribalMarkings.Add(TEXT("HandPalms_Yellow"));
            NewAppearance.TribalMarkings.Add(TEXT("FingerTips_Black"));
            break;
    }
    
    return NewAppearance;
}

void UChar_CretaceousTribalManager::UpdateTribalMarkings(APawn* TargetPawn, const TArray<FString>& Markings)
{
    if (!TargetPawn)
    {
        return;
    }
    
    // In a real implementation, this would apply decal materials or modify textures
    // For now, we'll log the markings being applied
    for (const FString& Marking : Markings)
    {
        UE_LOG(LogTemp, Log, TEXT("Applying tribal marking '%s' to %s"), *Marking, *TargetPawn->GetName());
    }
}

void UChar_CretaceousTribalManager::ApplyWeatheringEffects(APawn* TargetPawn, float WeatheringIntensity)
{
    if (!TargetPawn)
    {
        return;
    }
    
    // In a real implementation, this would modify material parameters for dirt, scars, etc.
    UE_LOG(LogTemp, Log, TEXT("Applying weathering effects (intensity: %.2f) to %s"), WeatheringIntensity, *TargetPawn->GetName());
}

void UChar_CretaceousTribalManager::SetTribalClothing(APawn* TargetPawn, const TArray<FString>& ClothingItems)
{
    if (!TargetPawn)
    {
        return;
    }
    
    // In a real implementation, this would attach clothing mesh components
    for (const FString& Clothing : ClothingItems)
    {
        UE_LOG(LogTemp, Log, TEXT("Equipping clothing '%s' on %s"), *Clothing, *TargetPawn->GetName());
    }
}

void UChar_CretaceousTribalManager::EquipTribalTools(APawn* TargetPawn, const TArray<FString>& Tools)
{
    if (!TargetPawn)
    {
        return;
    }
    
    // In a real implementation, this would attach weapon/tool mesh components
    for (const FString& Tool : Tools)
    {
        UE_LOG(LogTemp, Log, TEXT("Equipping tool '%s' on %s"), *Tool, *TargetPawn->GetName());
    }
}

void UChar_CretaceousTribalManager::SetTribalConfiguration(const FChar_CretaceousTribalConfig& NewConfig)
{
    TribalConfig = NewConfig;
    ValidateTribalConfiguration();
}

FChar_CretaceousTribalConfig UChar_CretaceousTribalManager::GetTribalConfiguration() const
{
    return TribalConfig;
}

int32 UChar_CretaceousTribalManager::GetActiveTribalNPCCount() const
{
    return ActiveTribalNPCs.Num();
}

TArray<APawn*> UChar_CretaceousTribalManager::GetAllTribalNPCs() const
{
    return ActiveTribalNPCs;
}

void UChar_CretaceousTribalManager::RemoveTribalNPC(APawn* NPCToRemove)
{
    if (NPCToRemove && ActiveTribalNPCs.Contains(NPCToRemove))
    {
        ActiveTribalNPCs.Remove(NPCToRemove);
        NPCToRemove->Destroy();
        
        UE_LOG(LogTemp, Log, TEXT("Removed tribal NPC: %s"), *NPCToRemove->GetName());
    }
}

void UChar_CretaceousTribalManager::ValidateTribalConfiguration()
{
    // Ensure reasonable limits
    TribalConfig.MaxTribalNPCs = FMath::Clamp(TribalConfig.MaxTribalNPCs, 1, 50);
    TribalConfig.SpawnRadius = FMath::Clamp(TribalConfig.SpawnRadius, 1000.0f, 50000.0f);
}

FChar_TribalAppearanceData UChar_CretaceousTribalManager::CreateTribalVariation(EChar_TribalRole Role)
{
    FChar_TribalAppearanceData Variation;
    Variation.TribalRole = Role;
    
    switch (Role)
    {
        case EChar_TribalRole::Hunter:
            Variation.CharacterName = TEXT("Hunter_Variant");
            Variation.BodyType = EChar_BodyType::Athletic;
            Variation.Age = 30.0f;
            Variation.ClothingItems.Add(TEXT("LeatherVest"));
            Variation.ClothingItems.Add(TEXT("FurBoots"));
            Variation.ToolsAndWeapons.Add(TEXT("StoneSpear"));
            Variation.ToolsAndWeapons.Add(TEXT("FlintKnife"));
            break;
            
        case EChar_TribalRole::Gatherer:
            Variation.CharacterName = TEXT("Gatherer_Variant");
            Variation.BodyType = EChar_BodyType::Lean;
            Variation.Age = 25.0f;
            Variation.ClothingItems.Add(TEXT("WovenSkirt"));
            Variation.ClothingItems.Add(TEXT("PlantFiberTop"));
            Variation.ToolsAndWeapons.Add(TEXT("GatheringBasket"));
            Variation.ToolsAndWeapons.Add(TEXT("DiggingStick"));
            break;
            
        case EChar_TribalRole::Crafter:
            Variation.CharacterName = TEXT("Crafter_Variant");
            Variation.BodyType = EChar_BodyType::Stocky;
            Variation.Age = 40.0f;
            Variation.ClothingItems.Add(TEXT("ToolBelt"));
            Variation.ClothingItems.Add(TEXT("WorkApron"));
            Variation.ToolsAndWeapons.Add(TEXT("StoneHammer"));
            Variation.ToolsAndWeapons.Add(TEXT("BoneAwl"));
            break;
            
        case EChar_TribalRole::Elder:
            Variation.CharacterName = TEXT("Elder_Variant");
            Variation.BodyType = EChar_BodyType::Lean;
            Variation.Age = 60.0f;
            Variation.ClothingItems.Add(TEXT("CeremonialRobe"));
            Variation.ClothingItems.Add(TEXT("BoneJewelry"));
            Variation.ToolsAndWeapons.Add(TEXT("WalkingStaff"));
            Variation.ToolsAndWeapons.Add(TEXT("MedicinePouch"));
            break;
    }
    
    return Variation;
}

void UChar_CretaceousTribalManager::CleanupInvalidNPCs()
{
    // Remove any NPCs that have been destroyed
    ActiveTribalNPCs.RemoveAll([](APawn* NPC)
    {
        return !IsValid(NPC);
    });
}