#include "World_CretaceousLandscapeManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Landscape/Landscape.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"
#include "LandscapeComponent.h"
#include "LandscapeInfo.h"

DEFINE_LOG_CATEGORY_STATIC(LogCretaceousLandscapeManager, Log, All);

UWorld_CretaceousLandscapeManager::UWorld_CretaceousLandscapeManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Initialize default values
    TargetLandscapeSize = FVector2D(10000.0f, 10000.0f); // 10km x 10km minimum
    CurrentLandscapeSize = FVector2D::ZeroVector;
    bIsLandscapeValid = false;
    bAutoManageLandscape = true;
    
    // Performance settings
    MaxLandscapeComponents = 1024;
    OptimalComponentSize = 127; // Quads per component
    
    // Biome configuration
    RequiredBiomeCount = 5;
    MinBiomeRadius = 800.0f;
    MaxBiomeRadius = 1500.0f;
}

void UWorld_CretaceousLandscapeManager::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoManageLandscape)
    {
        InitializeLandscapeManagement();
    }
}

void UWorld_CretaceousLandscapeManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Periodic validation every 5 seconds
    static float ValidationTimer = 0.0f;
    ValidationTimer += DeltaTime;
    
    if (ValidationTimer >= 5.0f)
    {
        ValidateLandscapeState();
        ValidationTimer = 0.0f;
    }
}

void UWorld_CretaceousLandscapeManager::InitializeLandscapeManagement()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogCretaceousLandscapeManager, Error, TEXT("No valid world for landscape management"));
        return;
    }
    
    // Find existing landscape
    DiscoverExistingLandscape();
    
    // Validate current setup
    ValidateLandscapeState();
    
    // Create or update landscape if needed
    if (!bIsLandscapeValid)
    {
        EnsureLandscapeExists();
    }
    
    UE_LOG(LogCretaceousLandscapeManager, Log, TEXT("Landscape management initialized"));
}

void UWorld_CretaceousLandscapeManager::DiscoverExistingLandscape()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Find all landscape actors
    TArray<AActor*> LandscapeActors;
    UGameplayStatics::GetAllActorsOfClass(World, ALandscape::StaticClass(), LandscapeActors);
    
    ManagedLandscapes.Empty();
    
    for (AActor* Actor : LandscapeActors)
    {
        if (ALandscape* Landscape = Cast<ALandscape>(Actor))
        {
            FWorld_LandscapeData LandscapeData;
            LandscapeData.LandscapeActor = Landscape;
            LandscapeData.LandscapeName = Landscape->GetActorLabel();
            LandscapeData.Location = Landscape->GetActorLocation();
            LandscapeData.Scale = Landscape->GetActorScale3D();
            
            // Calculate effective size
            LandscapeData.EffectiveSize = CalculateLandscapeSize(Landscape);
            
            ManagedLandscapes.Add(LandscapeData);
            
            UE_LOG(LogCretaceousLandscapeManager, Log, TEXT("Discovered landscape: %s, Size: %s"), 
                   *LandscapeData.LandscapeName, *LandscapeData.EffectiveSize.ToString());
        }
    }
    
    // Set primary landscape
    if (ManagedLandscapes.Num() > 0)
    {
        PrimaryLandscape = ManagedLandscapes[0].LandscapeActor;
        CurrentLandscapeSize = ManagedLandscapes[0].EffectiveSize;
    }
}

FVector2D UWorld_CretaceousLandscapeManager::CalculateLandscapeSize(ALandscape* Landscape) const
{
    if (!Landscape)
    {
        return FVector2D::ZeroVector;
    }
    
    // Get landscape info
    ULandscapeInfo* LandscapeInfo = Landscape->GetLandscapeInfo();
    if (!LandscapeInfo)
    {
        // Fallback: use actor scale
        FVector Scale = Landscape->GetActorScale3D();
        return FVector2D(Scale.X * 100.0f, Scale.Y * 100.0f); // Convert to meters
    }
    
    // Calculate from landscape components
    FIntRect ComponentBounds = LandscapeInfo->GetCurrentLevelLandscapeProxy()->GetBoundingRect();
    int32 ComponentSizeQuads = Landscape->ComponentSizeQuads;
    FVector Scale = Landscape->GetActorScale3D();
    
    float SizeX = ComponentBounds.Width() * ComponentSizeQuads * Scale.X;
    float SizeY = ComponentBounds.Height() * ComponentSizeQuads * Scale.Y;
    
    return FVector2D(SizeX, SizeY);
}

bool UWorld_CretaceousLandscapeManager::ValidateLandscapeState()
{
    bIsLandscapeValid = false;
    
    // Check if we have a primary landscape
    if (!PrimaryLandscape || !IsValid(PrimaryLandscape))
    {
        UE_LOG(LogCretaceousLandscapeManager, Warning, TEXT("No valid primary landscape"));
        return false;
    }
    
    // Check size requirements
    CurrentLandscapeSize = CalculateLandscapeSize(PrimaryLandscape);
    
    if (CurrentLandscapeSize.X < TargetLandscapeSize.X || CurrentLandscapeSize.Y < TargetLandscapeSize.Y)
    {
        UE_LOG(LogCretaceousLandscapeManager, Warning, 
               TEXT("Landscape size %s is below target %s"), 
               *CurrentLandscapeSize.ToString(), *TargetLandscapeSize.ToString());
        return false;
    }
    
    // Check biome zones
    if (!ValidateBiomeZones())
    {
        UE_LOG(LogCretaceousLandscapeManager, Warning, TEXT("Biome zone validation failed"));
        return false;
    }
    
    bIsLandscapeValid = true;
    UE_LOG(LogCretaceousLandscapeManager, Log, TEXT("Landscape state validation passed"));
    return true;
}

bool UWorld_CretaceousLandscapeManager::ValidateBiomeZones()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Find biome zone actors
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    TArray<AActor*> BiomeZones;
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetActorLabel().Contains(TEXT("BiomeZone_")))
        {
            BiomeZones.Add(Actor);
        }
    }
    
    if (BiomeZones.Num() < RequiredBiomeCount)
    {
        UE_LOG(LogCretaceousLandscapeManager, Warning, 
               TEXT("Found %d biome zones, required %d"), BiomeZones.Num(), RequiredBiomeCount);
        return false;
    }
    
    // Validate biome zone distribution
    TArray<FString> RequiredBiomes = {
        TEXT("Pantano_SW"), TEXT("Floresta_NW"), TEXT("Savana_Centro"), 
        TEXT("Deserto_E"), TEXT("Montanha_NE")
    };
    
    for (const FString& RequiredBiome : RequiredBiomes)
    {
        bool bFound = false;
        for (AActor* BiomeActor : BiomeZones)
        {
            if (BiomeActor->GetActorLabel().Contains(RequiredBiome))
            {
                bFound = true;
                break;
            }
        }
        
        if (!bFound)
        {
            UE_LOG(LogCretaceousLandscapeManager, Warning, 
                   TEXT("Required biome zone not found: %s"), *RequiredBiome);
            return false;
        }
    }
    
    return true;
}

bool UWorld_CretaceousLandscapeManager::EnsureLandscapeExists()
{
    if (PrimaryLandscape && IsValid(PrimaryLandscape))
    {
        // Landscape exists, check if it needs expansion
        if (CurrentLandscapeSize.X < TargetLandscapeSize.X || CurrentLandscapeSize.Y < TargetLandscapeSize.Y)
        {
            return ExpandLandscape();
        }
        return true;
    }
    
    // Create new landscape
    return CreateNewLandscape();
}

bool UWorld_CretaceousLandscapeManager::CreateNewLandscape()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Create landscape actor
    FActorSpawnParameters SpawnParams;
    SpawnParams.Name = FName(TEXT("MainLandscape_Cretaceous"));
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
    ALandscape* NewLandscape = World->SpawnActor<ALandscape>(
        ALandscape::StaticClass(),
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        SpawnParams
    );
    
    if (NewLandscape)
    {
        NewLandscape->SetActorLabel(TEXT("MainLandscape_Cretaceous"));
        
        // Calculate scale for target size
        FVector TargetScale = FVector(
            TargetLandscapeSize.X / 100.0f,
            TargetLandscapeSize.Y / 100.0f,
            100.0f
        );
        
        NewLandscape->SetActorScale3D(TargetScale);
        
        PrimaryLandscape = NewLandscape;
        CurrentLandscapeSize = TargetLandscapeSize;
        
        // Add to managed landscapes
        FWorld_LandscapeData LandscapeData;
        LandscapeData.LandscapeActor = NewLandscape;
        LandscapeData.LandscapeName = NewLandscape->GetActorLabel();
        LandscapeData.Location = NewLandscape->GetActorLocation();
        LandscapeData.Scale = NewLandscape->GetActorScale3D();
        LandscapeData.EffectiveSize = TargetLandscapeSize;
        
        ManagedLandscapes.Add(LandscapeData);
        
        UE_LOG(LogCretaceousLandscapeManager, Log, 
               TEXT("Created new landscape: %s with size %s"), 
               *NewLandscape->GetActorLabel(), *TargetLandscapeSize.ToString());
        
        return true;
    }
    
    UE_LOG(LogCretaceousLandscapeManager, Error, TEXT("Failed to create new landscape"));
    return false;
}

bool UWorld_CretaceousLandscapeManager::ExpandLandscape()
{
    if (!PrimaryLandscape)
    {
        return false;
    }
    
    // Calculate new scale
    FVector CurrentScale = PrimaryLandscape->GetActorScale3D();
    FVector NewScale = FVector(
        FMath::Max(CurrentScale.X, TargetLandscapeSize.X / 100.0f),
        FMath::Max(CurrentScale.Y, TargetLandscapeSize.Y / 100.0f),
        CurrentScale.Z
    );
    
    PrimaryLandscape->SetActorScale3D(NewScale);
    CurrentLandscapeSize = CalculateLandscapeSize(PrimaryLandscape);
    
    // Update managed landscape data
    for (FWorld_LandscapeData& LandscapeData : ManagedLandscapes)
    {
        if (LandscapeData.LandscapeActor == PrimaryLandscape)
        {
            LandscapeData.Scale = NewScale;
            LandscapeData.EffectiveSize = CurrentLandscapeSize;
            break;
        }
    }
    
    UE_LOG(LogCretaceousLandscapeManager, Log, 
           TEXT("Expanded landscape to size %s with scale %s"), 
           *CurrentLandscapeSize.ToString(), *NewScale.ToString());
    
    return true;
}

FWorld_LandscapeData UWorld_CretaceousLandscapeManager::GetPrimaryLandscapeData() const
{
    FWorld_LandscapeData DefaultData;
    
    if (!PrimaryLandscape)
    {
        return DefaultData;
    }
    
    for (const FWorld_LandscapeData& LandscapeData : ManagedLandscapes)
    {
        if (LandscapeData.LandscapeActor == PrimaryLandscape)
        {
            return LandscapeData;
        }
    }
    
    return DefaultData;
}

TArray<FWorld_LandscapeData> UWorld_CretaceousLandscapeManager::GetAllManagedLandscapes() const
{
    return ManagedLandscapes;
}

bool UWorld_CretaceousLandscapeManager::SetTargetLandscapeSize(FVector2D NewTargetSize)
{
    if (NewTargetSize.X <= 0.0f || NewTargetSize.Y <= 0.0f)
    {
        UE_LOG(LogCretaceousLandscapeManager, Warning, TEXT("Invalid target landscape size"));
        return false;
    }
    
    TargetLandscapeSize = NewTargetSize;
    
    // Re-validate with new target
    ValidateLandscapeState();
    
    // Expand if needed
    if (!bIsLandscapeValid)
    {
        EnsureLandscapeExists();
    }
    
    UE_LOG(LogCretaceousLandscapeManager, Log, 
           TEXT("Set new target landscape size: %s"), *TargetLandscapeSize.ToString());
    
    return true;
}

void UWorld_CretaceousLandscapeManager::RefreshLandscapeManagement()
{
    // Rediscover landscapes
    DiscoverExistingLandscape();
    
    // Re-validate
    ValidateLandscapeState();
    
    // Fix issues if found
    if (!bIsLandscapeValid)
    {
        EnsureLandscapeExists();
    }
    
    UE_LOG(LogCretaceousLandscapeManager, Log, TEXT("Landscape management refreshed"));
}