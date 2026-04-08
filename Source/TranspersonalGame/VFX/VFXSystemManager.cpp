#include "VFXSystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/SceneComponent.h"

AVFXSystemManager::AVFXSystemManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.016f; // 60 FPS base tick
    
    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("VFXManagerRoot"));
    
    // Initialize default performance settings
    MaxActiveVFX = 50;
    LODUpdateFrequency = 0.1f;
    HighQualityDistance = 20.0f;
    MediumQualityDistance = 50.0f;
}

void AVFXSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Get player reference
    PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    
    // Initialize VFX library
    InitializeVFXLibrary();
    
    // Preload critical VFX assets
    PreloadVFXAssets();
    
    UE_LOG(LogTemp, Log, TEXT("VFX System Manager initialized with %d effect definitions"), VFXLibrary.Num());
}

void AVFXSystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update LOD system
    LODUpdateTimer += DeltaTime;
    if (LODUpdateTimer >= LODUpdateFrequency)
    {
        UpdateVFXLOD();
        LODUpdateTimer = 0.0f;
    }
    
    // Cleanup inactive VFX periodically
    static float CleanupTimer = 0.0f;
    CleanupTimer += DeltaTime;
    if (CleanupTimer >= 5.0f) // Every 5 seconds
    {
        ClearInactiveVFX();
        CleanupTimer = 0.0f;
    }
}

void AVFXSystemManager::InitializeVFXLibrary()
{
    // ENVIRONMENTAL VFX
    FVFXDefinition FootstepDirt;
    FootstepDirt.EffectName = TEXT("Footstep_Dirt");
    FootstepDirt.Category = EVFXCategory::Environmental;
    FootstepDirt.MaxDistance = 30.0f;
    FootstepDirt.bIsLooping = false;
    FootstepDirt.EmissionIntensity = 1.0f;
    VFXLibrary.Add(FootstepDirt.EffectName, FootstepDirt);
    
    FVFXDefinition FootstepMud;
    FootstepMud.EffectName = TEXT("Footstep_Mud");
    FootstepMud.Category = EVFXCategory::Environmental;
    FootstepMud.MaxDistance = 25.0f;
    FootstepMud.bIsLooping = false;
    VFXLibrary.Add(FootstepMud.EffectName, FootstepMud);
    
    FVFXDefinition FoliageRustle;
    FoliageRustle.EffectName = TEXT("Foliage_Disturbance");
    FoliageRustle.Category = EVFXCategory::Environmental;
    FoliageRustle.MaxDistance = 40.0f;
    FoliageRustle.bIsLooping = false;
    VFXLibrary.Add(FoliageRustle.EffectName, FoliageRustle);
    
    FVFXDefinition DustCloud;
    DustCloud.EffectName = TEXT("Dust_Cloud");
    DustCloud.Category = EVFXCategory::Environmental;
    DustCloud.MaxDistance = 60.0f;
    DustCloud.bIsLooping = false;
    VFXLibrary.Add(DustCloud.EffectName, DustCloud);
    
    // CREATURE VFX
    FVFXDefinition BreathCold;
    BreathCold.EffectName = TEXT("Breath_Cold");
    BreathCold.Category = EVFXCategory::Creature;
    BreathCold.MaxDistance = 15.0f;
    BreathCold.bIsLooping = true;
    VFXLibrary.Add(BreathCold.EffectName, BreathCold);
    
    FVFXDefinition BloodSplatter;
    BloodSplatter.EffectName = TEXT("Blood_Splatter");
    BloodSplatter.Category = EVFXCategory::Creature;
    BloodSplatter.MaxDistance = 20.0f;
    BloodSplatter.bIsLooping = false;
    VFXLibrary.Add(BloodSplatter.EffectName, BloodSplatter);
    
    FVFXDefinition HeatShimmer;
    HeatShimmer.EffectName = TEXT("Heat_Distortion");
    HeatShimmer.Category = EVFXCategory::Creature;
    HeatShimmer.MaxDistance = 50.0f;
    HeatShimmer.bIsLooping = true;
    VFXLibrary.Add(HeatShimmer.EffectName, HeatShimmer);
    
    // SURVIVAL VFX
    FVFXDefinition Campfire;
    Campfire.EffectName = TEXT("Fire_Campfire");
    Campfire.Category = EVFXCategory::Survival;
    Campfire.MaxDistance = 80.0f;
    Campfire.bIsLooping = true;
    VFXLibrary.Add(Campfire.EffectName, Campfire);
    
    FVFXDefinition CraftingSparks;
    CraftingSparks.EffectName = TEXT("Sparks_Crafting");
    CraftingSparks.Category = EVFXCategory::Survival;
    CraftingSparks.MaxDistance = 25.0f;
    CraftingSparks.bIsLooping = false;
    VFXLibrary.Add(CraftingSparks.EffectName, CraftingSparks);
    
    FVFXDefinition Decomposition;
    Decomposition.EffectName = TEXT("Decomposition_Organic");
    Decomposition.Category = EVFXCategory::Survival;
    Decomposition.MaxDistance = 30.0f;
    Decomposition.bIsLooping = true;
    VFXLibrary.Add(Decomposition.EffectName, Decomposition);
    
    // ATMOSPHERIC VFX
    FVFXDefinition RainHeavy;
    RainHeavy.EffectName = TEXT("Rain_Heavy");
    RainHeavy.Category = EVFXCategory::Atmospheric;
    RainHeavy.MaxDistance = 200.0f;
    RainHeavy.bIsLooping = true;
    VFXLibrary.Add(RainHeavy.EffectName, RainHeavy);
    
    FVFXDefinition MorningFog;
    MorningFog.EffectName = TEXT("Fog_Morning");
    MorningFog.Category = EVFXCategory::Atmospheric;
    MorningFog.MaxDistance = 150.0f;
    MorningFog.bIsLooping = true;
    VFXLibrary.Add(MorningFog.EffectName, MorningFog);
    
    // INTERACTIVE VFX
    FVFXDefinition DiggingDirt;
    DiggingDirt.EffectName = TEXT("Digging_Dirt");
    DiggingDirt.Category = EVFXCategory::Interactive;
    DiggingDirt.MaxDistance = 20.0f;
    DiggingDirt.bIsLooping = false;
    VFXLibrary.Add(DiggingDirt.EffectName, DiggingDirt);
}

UNiagaraComponent* AVFXSystemManager::SpawnVFX(const FString& EffectName, FVector Location, FRotator Rotation, AActor* AttachToActor)
{
    // Check if we've reached max active VFX
    if (ActiveVFXComponents.Num() >= MaxActiveVFX)
    {
        UE_LOG(LogTemp, Warning, TEXT("Max active VFX reached (%d). Skipping spawn of %s"), MaxActiveVFX, *EffectName);
        return nullptr;
    }
    
    // Find VFX definition
    FVFXDefinition* VFXDef = VFXLibrary.Find(EffectName);
    if (!VFXDef)
    {
        UE_LOG(LogTemp, Error, TEXT("VFX Effect not found: %s"), *EffectName);
        return nullptr;
    }
    
    // Determine LOD level based on distance to player
    float DistanceToPlayer = 0.0f;
    if (PlayerPawn)
    {
        DistanceToPlayer = FVector::Dist(PlayerPawn->GetActorLocation(), Location);
    }
    
    EVFXQualityLevel QualityLevel = GetVFXQualityForDistance(DistanceToPlayer);
    UNiagaraSystem* NiagaraSystem = GetVFXSystemForLOD(EffectName, QualityLevel);
    
    if (!NiagaraSystem)
    {
        UE_LOG(LogTemp, Error, TEXT("No Niagara System found for %s at LOD %d"), *EffectName, (int32)QualityLevel);
        return nullptr;
    }
    
    // Spawn the VFX
    UNiagaraComponent* VFXComponent = nullptr;
    
    if (AttachToActor)
    {
        VFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
            NiagaraSystem,
            AttachToActor->GetRootComponent(),
            NAME_None,
            Location,
            Rotation,
            EAttachLocation::KeepWorldPosition,
            true
        );
    }
    else
    {
        VFXComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            NiagaraSystem,
            Location,
            Rotation
        );
    }
    
    if (VFXComponent)
    {
        // Generate unique instance ID
        FString InstanceID = GenerateVFXInstanceID();
        ActiveVFXComponents.Add(InstanceID, VFXComponent);
        
        // Set emission intensity
        VFXComponent->SetFloatParameter(TEXT("EmissionIntensity"), VFXDef->EmissionIntensity);
        
        UE_LOG(LogTemp, Log, TEXT("Spawned VFX: %s at location %s (LOD: %d)"), 
               *EffectName, *Location.ToString(), (int32)QualityLevel);
        
        return VFXComponent;
    }
    
    return nullptr;
}

void AVFXSystemManager::StopVFX(const FString& InstanceID)
{
    UNiagaraComponent** FoundComponent = ActiveVFXComponents.Find(InstanceID);
    if (FoundComponent && *FoundComponent)
    {
        (*FoundComponent)->Deactivate();
        CleanupVFXComponent(*FoundComponent);
        ActiveVFXComponents.Remove(InstanceID);
    }
}

void AVFXSystemManager::StopAllVFXByCategory(EVFXCategory Category)
{
    TArray<FString> ToRemove;
    
    for (auto& VFXPair : ActiveVFXComponents)
    {
        if (VFXPair.Value && IsValid(VFXPair.Value))
        {
            // Find the effect definition to check category
            for (auto& LibraryPair : VFXLibrary)
            {
                if (LibraryPair.Value.Category == Category)
                {
                    VFXPair.Value->Deactivate();
                    CleanupVFXComponent(VFXPair.Value);
                    ToRemove.Add(VFXPair.Key);
                    break;
                }
            }
        }
    }
    
    for (const FString& InstanceID : ToRemove)
    {
        ActiveVFXComponents.Remove(InstanceID);
    }
}

EVFXQualityLevel AVFXSystemManager::GetVFXQualityForDistance(float Distance)
{
    if (Distance <= HighQualityDistance)
    {
        return EVFXQualityLevel::High;
    }
    else if (Distance <= MediumQualityDistance)
    {
        return EVFXQualityLevel::Medium;
    }
    else
    {
        return EVFXQualityLevel::Low;
    }
}

void AVFXSystemManager::UpdateVFXLOD()
{
    if (!PlayerPawn)
        return;
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    for (auto& VFXPair : ActiveVFXComponents)
    {
        if (VFXPair.Value && IsValid(VFXPair.Value))
        {
            FVector VFXLocation = VFXPair.Value->GetComponentLocation();
            float Distance = FVector::Dist(PlayerLocation, VFXLocation);
            
            EVFXQualityLevel NewQuality = GetVFXQualityForDistance(Distance);
            
            // Update LOD if needed (this would require storing current LOD state)
            // For now, we'll handle this in future iterations
        }
    }
}

// Environmental VFX Implementation
void AVFXSystemManager::SpawnFootstepEffect(FVector Location, bool bIsLarge, bool bIsMuddy)
{
    FString EffectName = bIsMuddy ? TEXT("Footstep_Mud") : TEXT("Footstep_Dirt");
    UNiagaraComponent* VFX = SpawnVFX(EffectName, Location);
    
    if (VFX)
    {
        float SizeMultiplier = bIsLarge ? 2.0f : 1.0f;
        VFX->SetFloatParameter(TEXT("SizeMultiplier"), SizeMultiplier);
    }
}

void AVFXSystemManager::SpawnFoliageDisturbance(FVector Location, float Intensity)
{
    UNiagaraComponent* VFX = SpawnVFX(TEXT("Foliage_Disturbance"), Location);
    
    if (VFX)
    {
        VFX->SetFloatParameter(TEXT("DisturbanceIntensity"), Intensity);
    }
}

void AVFXSystemManager::SpawnDustCloud(FVector Location, FVector Direction, float Intensity)
{
    FRotator Rotation = Direction.Rotation();
    UNiagaraComponent* VFX = SpawnVFX(TEXT("Dust_Cloud"), Location, Rotation);
    
    if (VFX)
    {
        VFX->SetFloatParameter(TEXT("DustIntensity"), Intensity);
        VFX->SetVectorParameter(TEXT("WindDirection"), Direction);
    }
}

// Creature VFX Implementation
void AVFXSystemManager::SpawnBreathEffect(AActor* Creature, bool bIsVisible, float Temperature)
{
    if (!Creature)
        return;
    
    if (bIsVisible && Temperature < 15.0f) // Cold breath visible
    {
        UNiagaraComponent* VFX = SpawnVFX(TEXT("Breath_Cold"), FVector::ZeroVector, FRotator::ZeroRotator, Creature);
        
        if (VFX)
        {
            VFX->SetFloatParameter(TEXT("Temperature"), Temperature);
            VFX->SetFloatParameter(TEXT("Opacity"), FMath::Clamp((15.0f - Temperature) / 15.0f, 0.0f, 1.0f));
        }
    }
}

void AVFXSystemManager::SpawnBloodEffect(FVector Location, FVector Direction, bool bIsHeavyBleeding)
{
    FRotator Rotation = Direction.Rotation();
    UNiagaraComponent* VFX = SpawnVFX(TEXT("Blood_Splatter"), Location, Rotation);
    
    if (VFX)
    {
        float BloodAmount = bIsHeavyBleeding ? 2.0f : 1.0f;
        VFX->SetFloatParameter(TEXT("BloodAmount"), BloodAmount);
        VFX->SetVectorParameter(TEXT("ImpactDirection"), Direction);
    }
}

void AVFXSystemManager::SpawnHeatDistortion(AActor* LargeCreature, float Intensity)
{
    if (!LargeCreature)
        return;
    
    UNiagaraComponent* VFX = SpawnVFX(TEXT("Heat_Distortion"), FVector::ZeroVector, FRotator::ZeroRotator, LargeCreature);
    
    if (VFX)
    {
        VFX->SetFloatParameter(TEXT("HeatIntensity"), Intensity);
    }
}

// Survival VFX Implementation
UNiagaraComponent* AVFXSystemManager::SpawnFireEffect(FVector Location, float Intensity, bool bIsCampfire)
{
    UNiagaraComponent* VFX = SpawnVFX(TEXT("Fire_Campfire"), Location);
    
    if (VFX)
    {
        VFX->SetFloatParameter(TEXT("FireIntensity"), Intensity);
        VFX->SetBoolParameter(TEXT("IsCampfire"), bIsCampfire);
    }
    
    return VFX;
}

void AVFXSystemManager::SpawnSparksEffect(FVector Location, FVector Direction, float Intensity)
{
    FRotator Rotation = Direction.Rotation();
    UNiagaraComponent* VFX = SpawnVFX(TEXT("Sparks_Crafting"), Location, Rotation);
    
    if (VFX)
    {
        VFX->SetFloatParameter(TEXT("SparkIntensity"), Intensity);
        VFX->SetVectorParameter(TEXT("SparkDirection"), Direction);
    }
}

void AVFXSystemManager::SpawnDecompositionEffect(AActor* OrganicObject, float Stage)
{
    if (!OrganicObject)
        return;
    
    UNiagaraComponent* VFX = SpawnVFX(TEXT("Decomposition_Organic"), FVector::ZeroVector, FRotator::ZeroRotator, OrganicObject);
    
    if (VFX)
    {
        VFX->SetFloatParameter(TEXT("DecompositionStage"), Stage);
    }
}

// Atmospheric VFX Implementation
void AVFXSystemManager::SetRainIntensity(float Intensity)
{
    // This would control a global rain system
    // Implementation depends on weather system integration
    UE_LOG(LogTemp, Log, TEXT("Rain intensity set to: %f"), Intensity);
}

void AVFXSystemManager::SetFogDensity(float Density)
{
    // This would control global fog/mist
    UE_LOG(LogTemp, Log, TEXT("Fog density set to: %f"), Density);
}

void AVFXSystemManager::SpawnMorningMist(FVector Location, float Coverage)
{
    UNiagaraComponent* VFX = SpawnVFX(TEXT("Fog_Morning"), Location);
    
    if (VFX)
    {
        VFX->SetFloatParameter(TEXT("MistCoverage"), Coverage);
    }
}

// Interactive VFX Implementation
void AVFXSystemManager::SpawnDiggingEffect(FVector Location, float Intensity)
{
    UNiagaraComponent* VFX = SpawnVFX(TEXT("Digging_Dirt"), Location);
    
    if (VFX)
    {
        VFX->SetFloatParameter(TEXT("DiggingIntensity"), Intensity);
    }
}

void AVFXSystemManager::SpawnCraftingEffect(FVector Location, FString CraftingType)
{
    if (CraftingType.Contains(TEXT("Fire")) || CraftingType.Contains(TEXT("Metal")))
    {
        SpawnSparksEffect(Location, FVector::UpVector, 1.0f);
    }
    // Add more crafting-specific effects as needed
}

// Utility Functions
void AVFXSystemManager::PreloadVFXAssets()
{
    // This would preload critical VFX assets into memory
    // Implementation depends on asset streaming strategy
    UE_LOG(LogTemp, Log, TEXT("Preloading VFX assets..."));
}

void AVFXSystemManager::ClearInactiveVFX()
{
    TArray<FString> ToRemove;
    
    for (auto& VFXPair : ActiveVFXComponents)
    {
        if (!VFXPair.Value || !IsValid(VFXPair.Value) || !VFXPair.Value->IsActive())
        {
            if (VFXPair.Value)
            {
                CleanupVFXComponent(VFXPair.Value);
            }
            ToRemove.Add(VFXPair.Key);
        }
    }
    
    for (const FString& InstanceID : ToRemove)
    {
        ActiveVFXComponents.Remove(InstanceID);
    }
    
    if (ToRemove.Num() > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Cleaned up %d inactive VFX components"), ToRemove.Num());
    }
}

int32 AVFXSystemManager::GetActiveVFXCount()
{
    return ActiveVFXComponents.Num();
}

// Private Helper Functions
FString AVFXSystemManager::GenerateVFXInstanceID()
{
    static int32 InstanceCounter = 0;
    return FString::Printf(TEXT("VFX_%d_%f"), ++InstanceCounter, GetWorld()->GetTimeSeconds());
}

void AVFXSystemManager::CleanupVFXComponent(UNiagaraComponent* Component)
{
    if (Component && IsValid(Component))
    {
        Component->DestroyComponent();
    }
}

UNiagaraSystem* AVFXSystemManager::GetVFXSystemForLOD(const FString& EffectName, EVFXQualityLevel LOD)
{
    FVFXDefinition* VFXDef = VFXLibrary.Find(EffectName);
    if (!VFXDef)
        return nullptr;
    
    int32 LODIndex = (int32)LOD;
    if (VFXDef->LODSystems.IsValidIndex(LODIndex) && VFXDef->LODSystems[LODIndex].IsValid())
    {
        return VFXDef->LODSystems[LODIndex].LoadSynchronous();
    }
    
    // Fallback to any available LOD
    for (auto& LODSystem : VFXDef->LODSystems)
    {
        if (LODSystem.IsValid())
        {
            return LODSystem.LoadSynchronous();
        }
    }
    
    return nullptr;
}