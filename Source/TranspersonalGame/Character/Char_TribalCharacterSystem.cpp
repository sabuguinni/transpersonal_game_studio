#include "Char_TribalCharacterSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

AChar_TribalCharacter::AChar_TribalCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Set default collision
    GetCapsuleComponent()->SetCapsuleSize(42.0f, 96.0f);

    // Configure character movement
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
    GetCharacterMovement()->JumpZVelocity = 600.0f;
    GetCharacterMovement()->AirControl = 0.2f;
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

    // Initialize equipment components
    WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
    WeaponMesh->SetupAttachment(GetMesh(), TEXT("hand_r"));
    WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    ToolMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ToolMesh"));
    ToolMesh->SetupAttachment(GetMesh(), TEXT("pelvis"));
    ToolMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    AccessoryMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AccessoryMesh"));
    AccessoryMesh->SetupAttachment(GetMesh(), TEXT("head"));
    AccessoryMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Initialize default values
    RandomizeAppearance();
    InitializeComponents();
}

void AChar_TribalCharacter::BeginPlay()
{
    Super::BeginPlay();
    
    SetupDefaultMeshes();
    ApplyAppearanceToMesh();
    
    // Set movement speed based on role and age
    float SpeedMultiplier = 1.0f;
    switch(Appearance.AgeGroup)
    {
        case EChar_AgeGroup::YoungAdult:
            SpeedMultiplier = 1.1f;
            break;
        case EChar_AgeGroup::Adult:
            SpeedMultiplier = 1.0f;
            break;
        case EChar_AgeGroup::MiddleAged:
            SpeedMultiplier = 0.9f;
            break;
        case EChar_AgeGroup::Elder:
            SpeedMultiplier = 0.7f;
            break;
    }
    
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed * SpeedMultiplier;
}

void AChar_TribalCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AChar_TribalCharacter::InitializeComponents()
{
    // Set default skeletal mesh (will be replaced with proper tribal mesh)
    if (GetMesh())
    {
        GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -96.0f));
        GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
    }
}

void AChar_TribalCharacter::SetupDefaultMeshes()
{
    // Try to load UE5 mannequin as placeholder
    static ConstructorHelpers::FObjectFinder<USkeletalMesh> MannequinMesh(TEXT("/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple"));
    if (MannequinMesh.Succeeded())
    {
        GetMesh()->SetSkeletalMesh(MannequinMesh.Object);
    }
    
    // Setup basic weapon mesh (spear placeholder)
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SpearMesh(TEXT("/Engine/BasicShapes/Cylinder"));
    if (SpearMesh.Succeeded() && WeaponMesh)
    {
        WeaponMesh->SetStaticMesh(SpearMesh.Object);
        WeaponMesh->SetRelativeScale3D(FVector(0.05f, 0.05f, 2.0f));
        WeaponMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
    }
}

void AChar_TribalCharacter::ApplyAppearanceToMesh()
{
    if (!GetMesh() || !GetMesh()->GetSkeletalMeshAsset())
        return;

    // Create dynamic material instance for customization
    UMaterialInterface* BaseMaterial = GetMesh()->GetMaterial(0);
    if (BaseMaterial)
    {
        UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
        if (DynamicMaterial)
        {
            // Apply skin tone
            DynamicMaterial->SetScalarParameterValue(TEXT("SkinTone"), Appearance.SkinTone);
            
            // Apply age-based modifications
            float AgeMultiplier = 1.0f;
            switch(Appearance.AgeGroup)
            {
                case EChar_AgeGroup::YoungAdult:
                    AgeMultiplier = 0.9f;
                    break;
                case EChar_AgeGroup::Elder:
                    AgeMultiplier = 1.2f;
                    break;
                default:
                    AgeMultiplier = 1.0f;
                    break;
            }
            
            DynamicMaterial->SetScalarParameterValue(TEXT("Weathering"), AgeMultiplier);
            GetMesh()->SetMaterial(0, DynamicMaterial);
        }
    }
}

void AChar_TribalCharacter::RandomizeAppearance()
{
    // Randomize age group
    int32 AgeRoll = FMath::RandRange(0, 100);
    if (AgeRoll < 30)
        Appearance.AgeGroup = EChar_AgeGroup::YoungAdult;
    else if (AgeRoll < 70)
        Appearance.AgeGroup = EChar_AgeGroup::Adult;
    else if (AgeRoll < 90)
        Appearance.AgeGroup = EChar_AgeGroup::MiddleAged;
    else
        Appearance.AgeGroup = EChar_AgeGroup::Elder;

    // Randomize appearance features
    Appearance.SkinTone = FMath::RandRange(0.2f, 0.9f);
    Appearance.HairStyle = FMath::RandRange(0, 5);
    Appearance.FacialFeatures = FMath::RandRange(0, 10);
    Appearance.BodyBuild = FMath::RandRange(0.3f, 0.8f);
    Appearance.bHasScars = FMath::RandBool();
    Appearance.ClothingVariation = FMath::RandRange(0, 4);

    // Randomize gender
    bIsFemale = FMath::RandBool();

    // Set age based on age group
    switch(Appearance.AgeGroup)
    {
        case EChar_AgeGroup::YoungAdult:
            Age = FMath::RandRange(18, 25);
            break;
        case EChar_AgeGroup::Adult:
            Age = FMath::RandRange(26, 40);
            break;
        case EChar_AgeGroup::MiddleAged:
            Age = FMath::RandRange(41, 55);
            break;
        case EChar_AgeGroup::Elder:
            Age = FMath::RandRange(56, 70);
            break;
    }

    // Randomize stats based on age and role
    Stats.Strength = FMath::RandRange(30.0f, 80.0f);
    Stats.Agility = FMath::RandRange(30.0f, 80.0f);
    Stats.Intelligence = FMath::RandRange(40.0f, 90.0f);
    Stats.Survival = FMath::RandRange(50.0f, 95.0f);
    Stats.SocialSkill = FMath::RandRange(20.0f, 85.0f);

    // Adjust stats based on age
    if (Appearance.AgeGroup == EChar_AgeGroup::Elder)
    {
        Stats.Intelligence += 20.0f;
        Stats.Survival += 15.0f;
        Stats.Strength -= 15.0f;
        Stats.Agility -= 20.0f;
    }
    else if (Appearance.AgeGroup == EChar_AgeGroup::YoungAdult)
    {
        Stats.Agility += 10.0f;
        Stats.Intelligence -= 10.0f;
        Stats.Survival -= 10.0f;
    }

    // Clamp all stats to valid ranges
    Stats.Strength = FMath::Clamp(Stats.Strength, 10.0f, 100.0f);
    Stats.Agility = FMath::Clamp(Stats.Agility, 10.0f, 100.0f);
    Stats.Intelligence = FMath::Clamp(Stats.Intelligence, 10.0f, 100.0f);
    Stats.Survival = FMath::Clamp(Stats.Survival, 10.0f, 100.0f);
    Stats.SocialSkill = FMath::Clamp(Stats.SocialSkill, 10.0f, 100.0f);
}

void AChar_TribalCharacter::SetTribalRole(EChar_TribalRole NewRole)
{
    TribalRole = NewRole;

    // Adjust stats based on role
    switch(NewRole)
    {
        case EChar_TribalRole::Hunter:
            Stats.Strength += 10.0f;
            Stats.Agility += 15.0f;
            WalkSpeed = 180.0f;
            RunSpeed = 450.0f;
            break;
        case EChar_TribalRole::Gatherer:
            Stats.Intelligence += 10.0f;
            Stats.Survival += 15.0f;
            WalkSpeed = 160.0f;
            RunSpeed = 380.0f;
            break;
        case EChar_TribalRole::Crafter:
            Stats.Intelligence += 20.0f;
            WalkSpeed = 140.0f;
            RunSpeed = 350.0f;
            break;
        case EChar_TribalRole::Elder:
            Stats.Intelligence += 25.0f;
            Stats.SocialSkill += 20.0f;
            Stats.Survival += 20.0f;
            WalkSpeed = 120.0f;
            RunSpeed = 300.0f;
            break;
        case EChar_TribalRole::Scout:
            Stats.Agility += 20.0f;
            Stats.Intelligence += 10.0f;
            WalkSpeed = 200.0f;
            RunSpeed = 500.0f;
            break;
    }

    // Clamp stats after role adjustment
    Stats.Strength = FMath::Clamp(Stats.Strength, 10.0f, 100.0f);
    Stats.Agility = FMath::Clamp(Stats.Agility, 10.0f, 100.0f);
    Stats.Intelligence = FMath::Clamp(Stats.Intelligence, 10.0f, 100.0f);
    Stats.Survival = FMath::Clamp(Stats.Survival, 10.0f, 100.0f);
    Stats.SocialSkill = FMath::Clamp(Stats.SocialSkill, 10.0f, 100.0f);

    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    }
}

void AChar_TribalCharacter::EquipWeapon(UStaticMesh* WeaponStaticMesh)
{
    if (WeaponMesh && WeaponStaticMesh)
    {
        WeaponMesh->SetStaticMesh(WeaponStaticMesh);
    }
}

void AChar_TribalCharacter::SetAgeGroup(EChar_AgeGroup NewAgeGroup)
{
    Appearance.AgeGroup = NewAgeGroup;
    ApplyAppearanceToMesh();
}

FString AChar_TribalCharacter::GetCharacterDescription() const
{
    FString RoleString;
    switch(TribalRole)
    {
        case EChar_TribalRole::Hunter: RoleString = TEXT("Hunter"); break;
        case EChar_TribalRole::Gatherer: RoleString = TEXT("Gatherer"); break;
        case EChar_TribalRole::Crafter: RoleString = TEXT("Crafter"); break;
        case EChar_TribalRole::Elder: RoleString = TEXT("Elder"); break;
        case EChar_TribalRole::Scout: RoleString = TEXT("Scout"); break;
    }

    FString AgeString;
    switch(Appearance.AgeGroup)
    {
        case EChar_AgeGroup::YoungAdult: AgeString = TEXT("Young Adult"); break;
        case EChar_AgeGroup::Adult: AgeString = TEXT("Adult"); break;
        case EChar_AgeGroup::MiddleAged: AgeString = TEXT("Middle Aged"); break;
        case EChar_AgeGroup::Elder: AgeString = TEXT("Elder"); break;
    }

    FString GenderString = bIsFemale ? TEXT("Female") : TEXT("Male");

    return FString::Printf(TEXT("%s %s %s (%d years old) - %s"), 
        *GenderString, *AgeString, *RoleString, Age, *CharacterName);
}

// Manager Implementation
AChar_TribalCharacter* UChar_TribalCharacterManager::SpawnTribalCharacter(UWorld* World, FVector Location, EChar_TribalRole Role)
{
    if (!World)
        return nullptr;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AChar_TribalCharacter* NewCharacter = World->SpawnActor<AChar_TribalCharacter>(AChar_TribalCharacter::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
    
    if (NewCharacter)
    {
        NewCharacter->SetTribalRole(Role);
        NewCharacter->RandomizeAppearance();
        
        // Generate random name based on role and gender
        TArray<FString> MaleNames = {TEXT("Korg"), TEXT("Thak"), TEXT("Grok"), TEXT("Ugg"), TEXT("Brak"), TEXT("Zog"), TEXT("Rak")};
        TArray<FString> FemaleNames = {TEXT("Nara"), TEXT("Kira"), TEXT("Zara"), TEXT("Lira"), TEXT("Vera"), TEXT("Mira"), TEXT("Tara")};
        
        if (NewCharacter->bIsFemale)
        {
            NewCharacter->CharacterName = FemaleNames[FMath::RandRange(0, FemaleNames.Num() - 1)];
        }
        else
        {
            NewCharacter->CharacterName = MaleNames[FMath::RandRange(0, MaleNames.Num() - 1)];
        }
    }

    return NewCharacter;
}

void UChar_TribalCharacterManager::SpawnTribalGroup(UWorld* World, FVector CenterLocation, int32 GroupSize)
{
    if (!World || GroupSize <= 0)
        return;

    TArray<EChar_TribalRole> Roles = {
        EChar_TribalRole::Hunter,
        EChar_TribalRole::Gatherer,
        EChar_TribalRole::Crafter,
        EChar_TribalRole::Scout
    };

    for (int32 i = 0; i < GroupSize; i++)
    {
        // Distribute characters in a circle around center
        float Angle = (2.0f * PI * i) / GroupSize;
        float Radius = FMath::RandRange(200.0f, 500.0f);
        FVector SpawnLocation = CenterLocation + FVector(
            FMath::Cos(Angle) * Radius,
            FMath::Sin(Angle) * Radius,
            0.0f
        );

        // Ensure one elder per group if group size > 3
        EChar_TribalRole Role;
        if (i == 0 && GroupSize > 3)
        {
            Role = EChar_TribalRole::Elder;
        }
        else
        {
            Role = Roles[FMath::RandRange(0, Roles.Num() - 1)];
        }

        SpawnTribalCharacter(World, SpawnLocation, Role);
    }
}

TArray<AChar_TribalCharacter*> UChar_TribalCharacterManager::GetAllTribalCharacters(UWorld* World)
{
    TArray<AChar_TribalCharacter*> TribalCharacters;
    
    if (!World)
        return TribalCharacters;

    for (TActorIterator<AChar_TribalCharacter> ActorItr(World); ActorItr; ++ActorItr)
    {
        AChar_TribalCharacter* Character = *ActorItr;
        if (Character && IsValid(Character))
        {
            TribalCharacters.Add(Character);
        }
    }

    return TribalCharacters;
}

void UChar_TribalCharacterManager::RandomizeAllTribalCharacters(UWorld* World)
{
    TArray<AChar_TribalCharacter*> AllCharacters = GetAllTribalCharacters(World);
    
    for (AChar_TribalCharacter* Character : AllCharacters)
    {
        if (Character)
        {
            Character->RandomizeAppearance();
        }
    }
}