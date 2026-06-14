#include "Char_PrimitiveHuman.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"
#include "Materials/MaterialInstanceDynamic.h"

AChar_PrimitiveHuman::AChar_PrimitiveHuman()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize survival stats
    Health = 100.0f;
    Hunger = 75.0f;
    Thirst = 80.0f;
    Stamina = 100.0f;
    Fear = 0.0f;
    Warmth = 70.0f;

    // Character setup
    GetCapsuleComponent()->SetCapsuleSize(42.0f, 96.0f);
    
    // Movement setup for primitive human
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
    GetCharacterMovement()->JumpZVelocity = 420.0f;
    GetCharacterMovement()->AirControl = 0.2f;
    GetCharacterMovement()->MaxWalkSpeed = 400.0f;
    GetCharacterMovement()->MinAnalogWalkSpeed = 20.0f;
    GetCharacterMovement()->BrakingDecelerationWalking = 2000.0f;

    // Create equipment components
    CreateEquipmentComponents();

    // Initialize appearance defaults
    Appearance = FChar_TribalAppearance();
    Equipment = FChar_TribalEquipment();

    LastUpdateTime = 0.0f;
    bCharacterInitialized = false;
}

void AChar_PrimitiveHuman::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeTribalCharacter();
    bCharacterInitialized = true;
    LastUpdateTime = GetWorld()->GetTimeSeconds();
}

void AChar_PrimitiveHuman::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bCharacterInitialized)
    {
        UpdateSurvivalStats(DeltaTime);
    }
}

void AChar_PrimitiveHuman::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AChar_PrimitiveHuman::InitializeTribalCharacter()
{
    // Set default mesh if available
    USkeletalMesh* DefaultMesh = GetMesh()->GetSkeletalMeshAsset();
    if (!DefaultMesh)
    {
        // Try to load UE5 mannequin mesh
        USkeletalMesh* MannequinMesh = LoadObject<USkeletalMesh>(nullptr, TEXT("/Engine/Characters/Mannequins/Meshes/SKM_Quinn_Simple"));
        if (MannequinMesh)
        {
            GetMesh()->SetSkeletalMesh(MannequinMesh);
        }
    }

    // Position mesh
    GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));
    GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

    // Apply initial appearance
    ApplyTribalAppearance(Appearance);
    UpdateCharacterMaterials();
}

void AChar_PrimitiveHuman::CreateEquipmentComponents()
{
    // Create weapon mesh component
    WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
    WeaponMesh->SetupAttachment(GetMesh(), TEXT("hand_r"));
    WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Create shield mesh component
    ShieldMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShieldMesh"));
    ShieldMesh->SetupAttachment(GetMesh(), TEXT("hand_l"));
    ShieldMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Initialize jewelry meshes array
    JewelryMeshes.Empty();
    
    // Create basic jewelry components
    for (int32 i = 0; i < 3; i++)
    {
        FString ComponentName = FString::Printf(TEXT("JewelryMesh_%d"), i);
        UStaticMeshComponent* JewelryMesh = CreateDefaultSubobject<UStaticMeshComponent>(*ComponentName);
        JewelryMesh->SetupAttachment(GetMesh());
        JewelryMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        JewelryMeshes.Add(JewelryMesh);
    }
}

void AChar_PrimitiveHuman::ApplyTribalAppearance(const FChar_TribalAppearance& NewAppearance)
{
    Appearance = NewAppearance;
    
    // Update character name
    SetActorLabel(Appearance.CharacterName);
    
    // Apply appearance changes to mesh materials
    UpdateCharacterMaterials();
    
    UE_LOG(LogTemp, Log, TEXT("Applied tribal appearance to character: %s"), *Appearance.CharacterName);
}

void AChar_PrimitiveHuman::EquipTribalGear(const FChar_TribalEquipment& NewEquipment)
{
    Equipment = NewEquipment;
    
    // Equip primary weapon
    if (Equipment.PrimaryWeapon.IsValid() && WeaponMesh)
    {
        WeaponMesh->SetStaticMesh(Equipment.PrimaryWeapon.LoadSynchronous());
    }
    
    // Equip shield
    if (Equipment.Shield.IsValid() && ShieldMesh)
    {
        ShieldMesh->SetStaticMesh(Equipment.Shield.LoadSynchronous());
    }
    
    // Equip jewelry
    for (int32 i = 0; i < FMath::Min(Equipment.Jewelry.Num(), JewelryMeshes.Num()); i++)
    {
        if (Equipment.Jewelry[i].IsValid() && JewelryMeshes[i])
        {
            JewelryMeshes[i]->SetStaticMesh(Equipment.Jewelry[i].LoadSynchronous());
        }
    }
    
    // Apply clothing material
    if (Equipment.ClothingMaterial.IsValid())
    {
        UMaterialInterface* ClothingMat = Equipment.ClothingMaterial.LoadSynchronous();
        if (ClothingMat && GetMesh())
        {
            GetMesh()->SetMaterial(0, ClothingMat);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Equipped tribal gear on character"));
}

void AChar_PrimitiveHuman::UpdateSurvivalStats(float DeltaTime)
{
    ProcessSurvivalDecay(DeltaTime);
    
    // Clamp all stats to valid ranges
    Health = FMath::Clamp(Health, 0.0f, 100.0f);
    Hunger = FMath::Clamp(Hunger, 0.0f, 100.0f);
    Thirst = FMath::Clamp(Thirst, 0.0f, 100.0f);
    Stamina = FMath::Clamp(Stamina, 0.0f, 100.0f);
    Fear = FMath::Clamp(Fear, 0.0f, 100.0f);
    Warmth = FMath::Clamp(Warmth, 0.0f, 100.0f);
}

void AChar_PrimitiveHuman::ProcessSurvivalDecay(float DeltaTime)
{
    // Hunger decay - faster in cold environments
    float HungerDecayRate = 2.0f;
    if (Warmth < 30.0f)
    {
        HungerDecayRate *= 1.5f;
    }
    Hunger = FMath::Max(0.0f, Hunger - (HungerDecayRate * DeltaTime));
    
    // Thirst decay - faster in hot environments
    float ThirstDecayRate = 3.0f;
    if (Warmth > 80.0f)
    {
        ThirstDecayRate *= 1.3f;
    }
    Thirst = FMath::Max(0.0f, Thirst - (ThirstDecayRate * DeltaTime));
    
    // Stamina regeneration when not moving
    if (GetVelocity().Size() < 10.0f)
    {
        Stamina = FMath::Min(100.0f, Stamina + (20.0f * DeltaTime));
    }
    else
    {
        Stamina = FMath::Max(0.0f, Stamina - (15.0f * DeltaTime));
    }
    
    // Health effects from low survival stats
    if (Hunger < 20.0f || Thirst < 15.0f)
    {
        Health = FMath::Max(0.0f, Health - (5.0f * DeltaTime));
    }
    
    // Fear decay over time
    Fear = FMath::Max(0.0f, Fear - (10.0f * DeltaTime));
}

void AChar_PrimitiveHuman::ApplyEnvironmentalEffects(float Temperature, float Humidity, bool bIsNight)
{
    // Temperature affects warmth
    if (Temperature < 15.0f)
    {
        Warmth = FMath::Max(0.0f, Warmth - (10.0f * GetWorld()->GetDeltaSeconds()));
    }
    else if (Temperature > 35.0f)
    {
        Warmth = FMath::Min(100.0f, Warmth + (5.0f * GetWorld()->GetDeltaSeconds()));
    }
    
    // Night increases fear slightly
    if (bIsNight)
    {
        Fear = FMath::Min(100.0f, Fear + (2.0f * GetWorld()->GetDeltaSeconds()));
    }
    
    // High humidity affects thirst
    if (Humidity > 80.0f)
    {
        Thirst = FMath::Max(0.0f, Thirst - (1.0f * GetWorld()->GetDeltaSeconds()));
    }
}

void AChar_PrimitiveHuman::SetTribalMarkings(const TArray<FString>& Markings)
{
    Appearance.TribalMarkings = Markings;
    UpdateCharacterMaterials();
}

void AChar_PrimitiveHuman::UpdateCharacterMaterials()
{
    if (!GetMesh() || !GetMesh()->GetSkeletalMeshAsset())
    {
        return;
    }
    
    // Create dynamic material instance for customization
    UMaterialInterface* BaseMaterial = GetMesh()->GetMaterial(0);
    if (BaseMaterial)
    {
        UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
        if (DynamicMaterial)
        {
            // Apply skin tone
            DynamicMaterial->SetScalarParameterValue(TEXT("SkinTone"), Appearance.SkinTone);
            
            // Apply hair color
            DynamicMaterial->SetVectorParameterValue(TEXT("HairColor"), Appearance.HairColor);
            
            // Apply body build scaling
            DynamicMaterial->SetScalarParameterValue(TEXT("BodyBuild"), Appearance.BodyBuild);
            
            GetMesh()->SetMaterial(0, DynamicMaterial);
        }
    }
}

float AChar_PrimitiveHuman::GetOverallCondition() const
{
    return (Health + Hunger + Thirst + Stamina + Warmth - Fear) / 5.0f;
}

bool AChar_PrimitiveHuman::IsHealthy() const
{
    return Health > 70.0f && Hunger > 30.0f && Thirst > 25.0f;
}

bool AChar_PrimitiveHuman::NeedsFood() const
{
    return Hunger < 40.0f;
}

bool AChar_PrimitiveHuman::NeedsWater() const
{
    return Thirst < 30.0f;
}

bool AChar_PrimitiveHuman::IsExhausted() const
{
    return Stamina < 20.0f;
}

bool AChar_PrimitiveHuman::IsAfraid() const
{
    return Fear > 60.0f;
}