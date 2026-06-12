#include "Core_BiomePhysicsAdapter.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

UCore_BiomePhysicsAdapter::UCore_BiomePhysicsAdapter()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f; // Update biome physics twice per second
    
    BiomeManager = nullptr;
    CurrentBiome = EBiomeType::Grassland;
    PreviousBiome = EBiomeType::Grassland;
    
    // Initialize default physics values
    InitializeBiomePhysicsValues();
}

void UCore_BiomePhysicsAdapter::BeginPlay()
{
    Super::BeginPlay();
    
    CacheBiomeManager();
    InitializeBiomePhysicsMaterials();
    
    if (ValidateBiomePhysicsSetup())
    {
        UE_LOG(LogTemp, Log, TEXT("Core_BiomePhysicsAdapter: Successfully initialized"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_BiomePhysicsAdapter: Initialization incomplete"));
    }
}

void UCore_BiomePhysicsAdapter::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (BiomeManager)
    {
        AActor* Owner = GetOwner();
        if (Owner)
        {
            FVector OwnerLocation = Owner->GetActorLocation();
            EBiomeType NewBiome = BiomeManager->GetBiomeAtLocation(OwnerLocation);
            
            if (NewBiome != CurrentBiome)
            {
                UpdateBiomePhysics(NewBiome);
            }
        }
    }
}

void UCore_BiomePhysicsAdapter::InitializeBiomePhysicsValues()
{
    // Friction Values (0.0 = ice, 1.0 = normal, 2.0 = very rough)
    BiomeFrictionValues.Add(EBiomeType::Grassland, 1.0f);
    BiomeFrictionValues.Add(EBiomeType::Forest, 0.8f);
    BiomeFrictionValues.Add(EBiomeType::Desert, 0.6f);
    BiomeFrictionValues.Add(EBiomeType::Swamp, 0.3f);
    BiomeFrictionValues.Add(EBiomeType::Mountain, 1.4f);
    BiomeFrictionValues.Add(EBiomeType::River, 0.1f);
    BiomeFrictionValues.Add(EBiomeType::Cave, 1.2f);
    
    // Movement Speed Multipliers
    BiomeMovementMultipliers.Add(EBiomeType::Grassland, 1.0f);
    BiomeMovementMultipliers.Add(EBiomeType::Forest, 0.85f);
    BiomeMovementMultipliers.Add(EBiomeType::Desert, 0.9f);
    BiomeMovementMultipliers.Add(EBiomeType::Swamp, 0.6f);
    BiomeMovementMultipliers.Add(EBiomeType::Mountain, 0.7f);
    BiomeMovementMultipliers.Add(EBiomeType::River, 0.4f);
    BiomeMovementMultipliers.Add(EBiomeType::Cave, 0.8f);
    
    // Gravity Multipliers (for environmental effects)
    BiomeGravityMultipliers.Add(EBiomeType::Grassland, 1.0f);
    BiomeGravityMultipliers.Add(EBiomeType::Forest, 1.0f);
    BiomeGravityMultipliers.Add(EBiomeType::Desert, 1.0f);
    BiomeGravityMultipliers.Add(EBiomeType::Swamp, 0.95f);
    BiomeGravityMultipliers.Add(EBiomeType::Mountain, 1.05f);
    BiomeGravityMultipliers.Add(EBiomeType::River, 0.8f);
    BiomeGravityMultipliers.Add(EBiomeType::Cave, 1.0f);
    
    // Wind Resistance
    BiomeWindResistance.Add(EBiomeType::Grassland, 0.1f);
    BiomeWindResistance.Add(EBiomeType::Forest, 0.05f);
    BiomeWindResistance.Add(EBiomeType::Desert, 0.2f);
    BiomeWindResistance.Add(EBiomeType::Swamp, 0.08f);
    BiomeWindResistance.Add(EBiomeType::Mountain, 0.25f);
    BiomeWindResistance.Add(EBiomeType::River, 0.02f);
    BiomeWindResistance.Add(EBiomeType::Cave, 0.0f);
}

void UCore_BiomePhysicsAdapter::InitializeBiomePhysicsMaterials()
{
    for (const auto& BiomePair : BiomeFrictionValues)
    {
        EBiomeType BiomeType = BiomePair.Key;
        UPhysicalMaterial* Material = CreateBiomePhysicsMaterial(BiomeType);
        if (Material)
        {
            BiomePhysicsMaterials.Add(BiomeType, Material);
        }
    }
}

UPhysicalMaterial* UCore_BiomePhysicsAdapter::CreateBiomePhysicsMaterial(EBiomeType BiomeType)
{
    UPhysicalMaterial* Material = NewObject<UPhysicalMaterial>(this);
    if (Material)
    {
        ConfigurePhysicsMaterialForBiome(Material, BiomeType);
        return Material;
    }
    return nullptr;
}

void UCore_BiomePhysicsAdapter::ConfigurePhysicsMaterialForBiome(UPhysicalMaterial* Material, EBiomeType BiomeType)
{
    if (!Material) return;
    
    float Friction = BiomeFrictionValues.Contains(BiomeType) ? BiomeFrictionValues[BiomeType] : 1.0f;
    
    Material->Friction = Friction;
    Material->StaticFriction = Friction * 1.1f;
    Material->Restitution = FMath::Clamp(1.0f - Friction, 0.1f, 0.9f);
    
    // Biome-specific material properties
    switch (BiomeType)
    {
        case EBiomeType::Desert:
            Material->Density = 1.2f; // Sand density
            break;
        case EBiomeType::Swamp:
            Material->Density = 1.8f; // Mud density
            Material->Restitution = 0.1f; // Very low bounce
            break;
        case EBiomeType::Mountain:
            Material->Density = 2.5f; // Rock density
            Material->Restitution = 0.3f; // Rocky bounce
            break;
        case EBiomeType::River:
            Material->Density = 1.0f; // Water density
            Material->Restitution = 0.8f; // Water splash
            break;
        default:
            Material->Density = 1.0f;
            break;
    }
}

UPhysicalMaterial* UCore_BiomePhysicsAdapter::GetCurrentBiomePhysicsMaterial() const
{
    if (BiomePhysicsMaterials.Contains(CurrentBiome))
    {
        return BiomePhysicsMaterials[CurrentBiome];
    }
    return nullptr;
}

float UCore_BiomePhysicsAdapter::GetCurrentBiomeFriction() const
{
    return BiomeFrictionValues.Contains(CurrentBiome) ? BiomeFrictionValues[CurrentBiome] : 1.0f;
}

float UCore_BiomePhysicsAdapter::GetCurrentBiomeMovementMultiplier() const
{
    return BiomeMovementMultipliers.Contains(CurrentBiome) ? BiomeMovementMultipliers[CurrentBiome] : 1.0f;
}

float UCore_BiomePhysicsAdapter::GetCurrentBiomeGravityMultiplier() const
{
    return BiomeGravityMultipliers.Contains(CurrentBiome) ? BiomeGravityMultipliers[CurrentBiome] : 1.0f;
}

void UCore_BiomePhysicsAdapter::ApplyBiomePhysicsToActor(AActor* TargetActor)
{
    if (!TargetActor) return;
    
    // Apply physics material to all primitive components
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    TargetActor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    UPhysicalMaterial* BiomeMaterial = GetCurrentBiomePhysicsMaterial();
    
    for (UPrimitiveComponent* Component : PrimitiveComponents)
    {
        if (Component && BiomeMaterial)
        {
            Component->SetPhysMaterialOverride(BiomeMaterial);
        }
    }
    
    // Apply movement multiplier to characters
    ACharacter* Character = Cast<ACharacter>(TargetActor);
    if (Character && Character->GetCharacterMovement())
    {
        float BaseSpeed = 600.0f; // Default UE5 character speed
        float BiomeMultiplier = GetCurrentBiomeMovementMultiplier();
        float NewMaxSpeed = BaseSpeed * BiomeMultiplier;
        
        Character->GetCharacterMovement()->MaxWalkSpeed = NewMaxSpeed;
        Character->GetCharacterMovement()->MaxWalkSpeedCrouched = NewMaxSpeed * 0.5f;
    }
}

void UCore_BiomePhysicsAdapter::UpdateBiomePhysics(EBiomeType NewBiome)
{
    PreviousBiome = CurrentBiome;
    CurrentBiome = NewBiome;
    
    OnBiomeChanged(PreviousBiome, CurrentBiome);
    
    // Apply new physics to owner
    ApplyBiomePhysicsToActor(GetOwner());
    
    UE_LOG(LogTemp, Log, TEXT("Core_BiomePhysicsAdapter: Updated biome physics from %d to %d"), 
           (int32)PreviousBiome, (int32)CurrentBiome);
}

FVector UCore_BiomePhysicsAdapter::CalculateBiomeWindForce(const FVector& ActorVelocity) const
{
    float WindResistance = BiomeWindResistance.Contains(CurrentBiome) ? BiomeWindResistance[CurrentBiome] : 0.1f;
    
    // Simple wind resistance calculation
    FVector WindForce = -ActorVelocity * WindResistance * ActorVelocity.Size();
    return WindForce;
}

float UCore_BiomePhysicsAdapter::CalculateBiomeMovementPenalty(float BaseSpeed) const
{
    float Multiplier = GetCurrentBiomeMovementMultiplier();
    return BaseSpeed * (1.0f - Multiplier);
}

void UCore_BiomePhysicsAdapter::ApplyTerrainPhysics(UPrimitiveComponent* Component, const FVector& ImpactPoint)
{
    if (!Component) return;
    
    UPhysicalMaterial* BiomeMaterial = GetCurrentBiomePhysicsMaterial();
    if (BiomeMaterial)
    {
        Component->SetPhysMaterialOverride(BiomeMaterial);
    }
    
    // Apply biome-specific impact effects
    float ImpactForce = 100.0f * GetCurrentBiomeFriction();
    FVector ImpactDirection = FVector::UpVector * ImpactForce;
    
    if (Component->IsSimulatingPhysics())
    {
        Component->AddImpulseAtLocation(ImpactDirection, ImpactPoint);
    }
}

bool UCore_BiomePhysicsAdapter::IsTerrainSuitableForMovement(const FVector& Location, float Radius) const
{
    if (!BiomeManager) return true;
    
    EBiomeType BiomeAtLocation = BiomeManager->GetBiomeAtLocation(Location);
    float MovementMultiplier = BiomeMovementMultipliers.Contains(BiomeAtLocation) ? 
                               BiomeMovementMultipliers[BiomeAtLocation] : 1.0f;
    
    // Consider terrain unsuitable if movement is severely restricted
    return MovementMultiplier > 0.3f;
}

void UCore_BiomePhysicsAdapter::OnBiomeChanged(EBiomeType OldBiome, EBiomeType NewBiome)
{
    UE_LOG(LogTemp, Log, TEXT("Core_BiomePhysicsAdapter: Biome transition from %d to %d"), 
           (int32)OldBiome, (int32)NewBiome);
    
    // Trigger any biome transition effects here
    // This could include particle effects, sound changes, etc.
}

void UCore_BiomePhysicsAdapter::CacheBiomeManager()
{
    if (!BiomeManager)
    {
        UWorld* World = GetWorld();
        if (World)
        {
            BiomeManager = World->GetGameInstance()->GetSubsystem<UBiomeManager>();
            if (!BiomeManager)
            {
                // Try to find BiomeManager as an actor component in the world
                TArray<AActor*> FoundActors;
                UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), FoundActors);
                
                for (AActor* Actor : FoundActors)
                {
                    UBiomeManager* FoundBiomeManager = Actor->FindComponentByClass<UBiomeManager>();
                    if (FoundBiomeManager)
                    {
                        BiomeManager = FoundBiomeManager;
                        break;
                    }
                }
            }
        }
    }
}

bool UCore_BiomePhysicsAdapter::ValidateBiomePhysicsSetup() const
{
    bool bIsValid = true;
    
    if (BiomeFrictionValues.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_BiomePhysicsAdapter: No biome friction values configured"));
        bIsValid = false;
    }
    
    if (BiomeMovementMultipliers.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_BiomePhysicsAdapter: No biome movement multipliers configured"));
        bIsValid = false;
    }
    
    if (!BiomeManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_BiomePhysicsAdapter: BiomeManager not found"));
        bIsValid = false;
    }
    
    return bIsValid;
}