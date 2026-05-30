#include "Char_TribalHumanSystem.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

AChar_TribalHuman::AChar_TribalHuman()
{
    PrimaryActorTick.bCanEverTick = true;

    // Set default capsule size
    GetCapsuleComponent()->SetCapsuleHalfHeight(88.0f);
    GetCapsuleComponent()->SetCapsuleRadius(34.0f);

    // Configure character movement
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
    GetCharacterMovement()->JumpZVelocity = 420.0f;
    GetCharacterMovement()->AirControl = 0.2f;
    GetCharacterMovement()->MaxWalkSpeed = 300.0f;

    // Create tool component
    ToolComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ToolComponent"));
    ToolComponent->SetupAttachment(GetMesh(), TEXT("hand_rSocket"));
    ToolComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Create accessory component
    AccessoryComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AccessoryComponent"));
    AccessoryComponent->SetupAttachment(GetMesh(), TEXT("spine_03"));
    AccessoryComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Initialize survival stats
    Hunger = 100.0f;
    Thirst = 100.0f;
    Warmth = 100.0f;
    Fatigue = 0.0f;

    // Initialize skills based on role
    HuntingSkill = 50.0f;
    GatheringSkill = 50.0f;
    CraftingSkill = 50.0f;
    SurvivalSkill = 50.0f;

    // Set default appearance
    Appearance.Role = EChar_TribalRole::Hunter;
    Appearance.ClothingType = EChar_TribalClothing::AnimalHide;
    Appearance.SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
    Appearance.Age = 25.0f;
    Appearance.MuscleMass = 0.7f;
    Appearance.bHasScars = true;
    Appearance.bHasTattoos = false;
}

void AChar_TribalHuman::BeginPlay()
{
    Super::BeginPlay();
    
    ApplyRoleBasedStats();
}

void AChar_TribalHuman::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateSurvivalStats(DeltaTime);
}

void AChar_TribalHuman::SetTribalAppearance(const FChar_TribalAppearance& NewAppearance)
{
    Appearance = NewAppearance;
    ApplyRoleBasedStats();
}

void AChar_TribalHuman::ApplyRoleBasedStats()
{
    switch (Appearance.Role)
    {
        case EChar_TribalRole::Hunter:
            HuntingSkill = FMath::RandRange(70.0f, 90.0f);
            SurvivalSkill = FMath::RandRange(60.0f, 80.0f);
            GetCharacterMovement()->MaxWalkSpeed = 350.0f;
            break;
            
        case EChar_TribalRole::Gatherer:
            GatheringSkill = FMath::RandRange(70.0f, 90.0f);
            SurvivalSkill = FMath::RandRange(50.0f, 70.0f);
            GetCharacterMovement()->MaxWalkSpeed = 280.0f;
            break;
            
        case EChar_TribalRole::Crafter:
            CraftingSkill = FMath::RandRange(80.0f, 95.0f);
            GatheringSkill = FMath::RandRange(60.0f, 75.0f);
            GetCharacterMovement()->MaxWalkSpeed = 250.0f;
            break;
            
        case EChar_TribalRole::Scout:
            SurvivalSkill = FMath::RandRange(80.0f, 95.0f);
            HuntingSkill = FMath::RandRange(60.0f, 75.0f);
            GetCharacterMovement()->MaxWalkSpeed = 400.0f;
            break;
            
        case EChar_TribalRole::Elder:
            CraftingSkill = FMath::RandRange(90.0f, 100.0f);
            SurvivalSkill = FMath::RandRange(85.0f, 100.0f);
            GetCharacterMovement()->MaxWalkSpeed = 200.0f;
            break;
            
        case EChar_TribalRole::Shaman:
            GatheringSkill = FMath::RandRange(75.0f, 90.0f);
            CraftingSkill = FMath::RandRange(70.0f, 85.0f);
            GetCharacterMovement()->MaxWalkSpeed = 220.0f;
            break;
    }
}

void AChar_TribalHuman::EquipTribalTool(UStaticMesh* ToolMesh)
{
    if (ToolComponent && ToolMesh)
    {
        ToolComponent->SetStaticMesh(ToolMesh);
    }
}

void AChar_TribalHuman::UpdateSurvivalStats(float DeltaTime)
{
    // Hunger decreases over time
    Hunger = FMath::Clamp(Hunger - (DeltaTime * 0.5f), 0.0f, 100.0f);
    
    // Thirst decreases faster than hunger
    Thirst = FMath::Clamp(Thirst - (DeltaTime * 0.8f), 0.0f, 100.0f);
    
    // Warmth affected by environment (simplified)
    float EnvironmentTemp = 50.0f; // Base temperature
    float TempDifference = FMath::Abs(Warmth - EnvironmentTemp);
    Warmth = FMath::Clamp(Warmth + ((EnvironmentTemp - Warmth) * DeltaTime * 0.1f), 0.0f, 100.0f);
    
    // Fatigue increases with activity
    float MovementSpeed = GetVelocity().Size();
    float FatigueRate = MovementSpeed > 100.0f ? 0.3f : 0.1f;
    Fatigue = FMath::Clamp(Fatigue + (DeltaTime * FatigueRate), 0.0f, 100.0f);
    
    // Health affected by survival stats
    if (IsStarving() || IsDehydrated() || IsExhausted())
    {
        float HealthLoss = DeltaTime * 2.0f;
        float CurrentHealth = GetHealth();
        SetHealth(FMath::Clamp(CurrentHealth - HealthLoss, 0.0f, GetMaxHealth()));
    }
}

void AChar_TribalHuman::GainExperience(EChar_TribalRole SkillType, float Amount)
{
    switch (SkillType)
    {
        case EChar_TribalRole::Hunter:
            HuntingSkill = FMath::Clamp(HuntingSkill + Amount, 0.0f, 100.0f);
            break;
        case EChar_TribalRole::Gatherer:
            GatheringSkill = FMath::Clamp(GatheringSkill + Amount, 0.0f, 100.0f);
            break;
        case EChar_TribalRole::Crafter:
            CraftingSkill = FMath::Clamp(CraftingSkill + Amount, 0.0f, 100.0f);
            break;
        case EChar_TribalRole::Scout:
            SurvivalSkill = FMath::Clamp(SurvivalSkill + Amount, 0.0f, 100.0f);
            break;
        default:
            break;
    }
}

FString AChar_TribalHuman::GetRoleDescription() const
{
    switch (Appearance.Role)
    {
        case EChar_TribalRole::Hunter:
            return TEXT("Skilled in tracking and hunting dangerous prey");
        case EChar_TribalRole::Gatherer:
            return TEXT("Expert at finding edible plants and resources");
        case EChar_TribalRole::Crafter:
            return TEXT("Master of creating tools and shelter");
        case EChar_TribalRole::Scout:
            return TEXT("Swift explorer of unknown territories");
        case EChar_TribalRole::Elder:
            return TEXT("Wise keeper of tribal knowledge");
        case EChar_TribalRole::Shaman:
            return TEXT("Healer and spiritual guide of the tribe");
        default:
            return TEXT("Unknown role");
    }
}

// Subsystem Implementation
void UChar_TribalHumanSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    SpawnedTribalHumans.Empty();
}

void UChar_TribalHumanSubsystem::Deinitialize()
{
    SpawnedTribalHumans.Empty();
    Super::Deinitialize();
}

AChar_TribalHuman* UChar_TribalHumanSubsystem::SpawnTribalHuman(const FVector& Location, const FRotator& Rotation, EChar_TribalRole Role)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AChar_TribalHuman* NewTribalHuman = World->SpawnActor<AChar_TribalHuman>(AChar_TribalHuman::StaticClass(), Location, Rotation, SpawnParams);
    
    if (NewTribalHuman)
    {
        FChar_TribalAppearance NewAppearance = GenerateRandomAppearance(Role);
        NewTribalHuman->SetTribalAppearance(NewAppearance);
        
        SpawnedTribalHumans.Add(NewTribalHuman);
    }

    return NewTribalHuman;
}

void UChar_TribalHumanSubsystem::SpawnTribalGroup(const FVector& CenterLocation, int32 GroupSize, float SpreadRadius)
{
    for (int32 i = 0; i < GroupSize; i++)
    {
        FVector RandomOffset = FVector(
            FMath::RandRange(-SpreadRadius, SpreadRadius),
            FMath::RandRange(-SpreadRadius, SpreadRadius),
            0.0f
        );
        
        FVector SpawnLocation = CenterLocation + RandomOffset;
        FRotator SpawnRotation = FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f);
        
        EChar_TribalRole RandomRole = static_cast<EChar_TribalRole>(FMath::RandRange(0, 5));
        
        SpawnTribalHuman(SpawnLocation, SpawnRotation, RandomRole);
    }
}

TArray<AChar_TribalHuman*> UChar_TribalHumanSubsystem::GetAllTribalHumans()
{
    // Clean up null references
    SpawnedTribalHumans.RemoveAll([](AChar_TribalHuman* Human) {
        return !IsValid(Human);
    });
    
    return SpawnedTribalHumans;
}

void UChar_TribalHumanSubsystem::DistributeTribalHumansAcrossBiomes()
{
    // Biome coordinates from memory
    TArray<FVector> BiomeLocations = {
        FVector(0, 0, 100),           // Savana
        FVector(-50000, -45000, 100), // Pantano
        FVector(-45000, 40000, 100),  // Floresta
        FVector(55000, 0, 100),       // Deserto
        FVector(40000, 50000, 100)    // Montanha
    };

    TArray<EChar_TribalRole> Roles = {
        EChar_TribalRole::Hunter,
        EChar_TribalRole::Gatherer,
        EChar_TribalRole::Crafter,
        EChar_TribalRole::Scout,
        EChar_TribalRole::Elder,
        EChar_TribalRole::Shaman
    };

    for (const FVector& BiomeCenter : BiomeLocations)
    {
        // Spawn 2-3 tribal humans per biome
        int32 HumansPerBiome = FMath::RandRange(2, 3);
        
        for (int32 i = 0; i < HumansPerBiome; i++)
        {
            FVector RandomOffset = FVector(
                FMath::RandRange(-5000.0f, 5000.0f),
                FMath::RandRange(-5000.0f, 5000.0f),
                0.0f
            );
            
            FVector SpawnLocation = BiomeCenter + RandomOffset;
            FRotator SpawnRotation = FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f);
            
            EChar_TribalRole RandomRole = Roles[FMath::RandRange(0, Roles.Num() - 1)];
            
            SpawnTribalHuman(SpawnLocation, SpawnRotation, RandomRole);
        }
    }
}

FChar_TribalAppearance UChar_TribalHumanSubsystem::GenerateRandomAppearance(EChar_TribalRole Role)
{
    FChar_TribalAppearance NewAppearance;
    
    NewAppearance.Role = Role;
    NewAppearance.ClothingType = static_cast<EChar_TribalClothing>(FMath::RandRange(0, 4));
    
    // Generate varied skin tones
    float SkinVariation = FMath::RandRange(0.3f, 0.9f);
    NewAppearance.SkinTone = FLinearColor(SkinVariation, SkinVariation * 0.8f, SkinVariation * 0.6f, 1.0f);
    
    // Age based on role
    switch (Role)
    {
        case EChar_TribalRole::Elder:
            NewAppearance.Age = FMath::RandRange(50.0f, 70.0f);
            break;
        case EChar_TribalRole::Scout:
            NewAppearance.Age = FMath::RandRange(18.0f, 30.0f);
            break;
        default:
            NewAppearance.Age = FMath::RandRange(20.0f, 45.0f);
            break;
    }
    
    NewAppearance.MuscleMass = FMath::RandRange(0.5f, 0.9f);
    NewAppearance.bHasScars = FMath::RandBool();
    NewAppearance.bHasTattoos = FMath::RandRange(0.0f, 1.0f) < 0.3f; // 30% chance
    
    return NewAppearance;
}