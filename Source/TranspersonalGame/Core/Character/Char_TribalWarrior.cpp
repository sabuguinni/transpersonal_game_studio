#include "Char_TribalWarrior.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

AChar_TribalWarrior::AChar_TribalWarrior()
{
    PrimaryActorTick.bCanEverTick = true;

    // Set default character properties
    GetCapsuleComponent()->SetCapsuleHalfHeight(88.0f);
    GetCapsuleComponent()->SetCapsuleRadius(34.0f);

    // Configure character movement
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
    GetCharacterMovement()->JumpZVelocity = 600.0f;
    GetCharacterMovement()->AirControl = 0.2f;
    GetCharacterMovement()->MaxWalkSpeed = 400.0f;
    GetCharacterMovement()->MinAnalogWalkSpeed = 20.0f;
    GetCharacterMovement()->BrakingDecelerationWalking = 2000.0f;

    // Don't rotate when the controller rotates
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // Create equipment components
    WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
    WeaponMesh->SetupAttachment(GetMesh(), TEXT("hand_rSocket"));
    WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    ShieldMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShieldMesh"));
    ShieldMesh->SetupAttachment(GetMesh(), TEXT("hand_lSocket"));
    ShieldMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    BackpackMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BackpackMesh"));
    BackpackMesh->SetupAttachment(GetMesh(), TEXT("spine_03"));
    BackpackMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Initialize default stats
    TribalStats.Health = 100.0f;
    TribalStats.Stamina = 100.0f;
    TribalStats.Hunger = 100.0f;
    TribalStats.Thirst = 100.0f;
    TribalStats.CombatExperience = 0.0f;
    TribalStats.CraftingSkill = 0.0f;
    TribalStats.SurvivalInstinct = 50.0f;

    // Set default state
    CurrentState = EChar_WarriorState::Idle;
    EquippedWeapon = EChar_WeaponType::None;

    // Try to load default mannequin mesh if available
    static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshAsset(TEXT("/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple"));
    if (MeshAsset.Succeeded())
    {
        GetMesh()->SetSkeletalMesh(MeshAsset.Object);
        GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f));
        GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
    }
}

void AChar_TribalWarrior::BeginPlay()
{
    Super::BeginPlay();
    
    UpdateCharacterAppearance();
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            FString::Printf(TEXT("Tribal Warrior %s spawned with %s weapon"), 
            *GetName(), 
            *UEnum::GetValueAsString(EquippedWeapon)));
    }
}

void AChar_TribalWarrior::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateSurvivalStats(DeltaTime);
    HandleStateTransitions();
    UpdateAnimationState();
}

void AChar_TribalWarrior::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    
    // Input bindings would be set up here for player-controlled characters
    // For NPCs, this can remain empty
}

void AChar_TribalWarrior::EquipWeapon(EChar_WeaponType WeaponType)
{
    EquippedWeapon = WeaponType;
    ApplyWeaponMesh(WeaponType);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, 
            FString::Printf(TEXT("%s equipped %s"), 
            *GetName(), 
            *UEnum::GetValueAsString(WeaponType)));
    }
}

void AChar_TribalWarrior::UnequipWeapon()
{
    EquippedWeapon = EChar_WeaponType::None;
    
    if (WeaponMesh)
    {
        WeaponMesh->SetStaticMesh(nullptr);
        WeaponMesh->SetVisibility(false);
    }
}

void AChar_TribalWarrior::SetWarriorState(EChar_WarriorState NewState)
{
    if (CurrentState != NewState)
    {
        EChar_WarriorState PreviousState = CurrentState;
        CurrentState = NewState;
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, 
                FString::Printf(TEXT("%s state changed from %s to %s"), 
                *GetName(), 
                *UEnum::GetValueAsString(PreviousState),
                *UEnum::GetValueAsString(NewState)));
        }
    }
}

void AChar_TribalWarrior::UpdateSurvivalStats(float DeltaTime)
{
    // Gradual stat degradation over time
    const float HungerDecayRate = 2.0f; // per minute
    const float ThirstDecayRate = 3.0f; // per minute
    const float StaminaRecoveryRate = 15.0f; // per minute when resting
    
    // Apply hunger and thirst decay
    TribalStats.Hunger = FMath::Max(0.0f, TribalStats.Hunger - (HungerDecayRate * DeltaTime / 60.0f));
    TribalStats.Thirst = FMath::Max(0.0f, TribalStats.Thirst - (ThirstDecayRate * DeltaTime / 60.0f));
    
    // Stamina recovery when resting
    if (CurrentState == EChar_WarriorState::Resting)
    {
        TribalStats.Stamina = FMath::Min(100.0f, TribalStats.Stamina + (StaminaRecoveryRate * DeltaTime / 60.0f));
    }
    
    // Health degradation from starvation/dehydration
    if (IsStarving() || IsDehydrated())
    {
        const float HealthDecayRate = 5.0f; // per minute when starving/dehydrated
        TribalStats.Health = FMath::Max(0.0f, TribalStats.Health - (HealthDecayRate * DeltaTime / 60.0f));
    }
}

void AChar_TribalWarrior::TakeDamage(float DamageAmount)
{
    TribalStats.Health = FMath::Max(0.0f, TribalStats.Health - DamageAmount);
    
    if (TribalStats.Health <= 0.0f)
    {
        SetWarriorState(EChar_WarriorState::Wounded);
    }
    else if (TribalStats.Health < 30.0f)
    {
        SetWarriorState(EChar_WarriorState::Wounded);
    }
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
            FString::Printf(TEXT("%s took %.1f damage, Health: %.1f"), 
            *GetName(), DamageAmount, TribalStats.Health));
    }
}

void AChar_TribalWarrior::RestoreHealth(float HealAmount)
{
    TribalStats.Health = FMath::Min(100.0f, TribalStats.Health + HealAmount);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, 
            FString::Printf(TEXT("%s healed %.1f, Health: %.1f"), 
            *GetName(), HealAmount, TribalStats.Health));
    }
}

void AChar_TribalWarrior::ConsumeFood(float FoodValue)
{
    TribalStats.Hunger = FMath::Min(100.0f, TribalStats.Hunger + FoodValue);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, 
            FString::Printf(TEXT("%s ate food (+%.1f), Hunger: %.1f"), 
            *GetName(), FoodValue, TribalStats.Hunger));
    }
}

void AChar_TribalWarrior::DrinkWater(float WaterValue)
{
    TribalStats.Thirst = FMath::Min(100.0f, TribalStats.Thirst + WaterValue);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, 
            FString::Printf(TEXT("%s drank water (+%.1f), Thirst: %.1f"), 
            *GetName(), WaterValue, TribalStats.Thirst));
    }
}

void AChar_TribalWarrior::GainCombatExperience(float ExperienceAmount)
{
    TribalStats.CombatExperience = FMath::Min(100.0f, TribalStats.CombatExperience + ExperienceAmount);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Purple, 
            FString::Printf(TEXT("%s gained combat experience (+%.1f), Total: %.1f"), 
            *GetName(), ExperienceAmount, TribalStats.CombatExperience));
    }
}

void AChar_TribalWarrior::ImproveCraftingSkill(float SkillAmount)
{
    TribalStats.CraftingSkill = FMath::Min(100.0f, TribalStats.CraftingSkill + SkillAmount);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, 
            FString::Printf(TEXT("%s improved crafting (+%.1f), Skill: %.1f"), 
            *GetName(), SkillAmount, TribalStats.CraftingSkill));
    }
}

void AChar_TribalWarrior::ApplySkinMaterial(UMaterialInterface* NewSkinMaterial)
{
    if (NewSkinMaterial && GetMesh())
    {
        SkinMaterial = NewSkinMaterial;
        GetMesh()->SetMaterial(0, SkinMaterial);
    }
}

void AChar_TribalWarrior::ApplyClothingMaterial(UMaterialInterface* NewClothingMaterial)
{
    if (NewClothingMaterial && GetMesh())
    {
        ClothingMaterial = NewClothingMaterial;
        GetMesh()->SetMaterial(1, ClothingMaterial);
    }
}

void AChar_TribalWarrior::AddTattoo(UMaterialInterface* TattooMaterial)
{
    if (TattooMaterial)
    {
        TattooMaterials.Add(TattooMaterial);
        UpdateCharacterAppearance();
    }
}

void AChar_TribalWarrior::RemoveAllTattoos()
{
    TattooMaterials.Empty();
    UpdateCharacterAppearance();
}

bool AChar_TribalWarrior::IsAlive() const
{
    return TribalStats.Health > 0.0f;
}

bool AChar_TribalWarrior::IsWounded() const
{
    return TribalStats.Health < 30.0f;
}

bool AChar_TribalWarrior::IsStarving() const
{
    return TribalStats.Hunger < 20.0f;
}

bool AChar_TribalWarrior::IsDehydrated() const
{
    return TribalStats.Thirst < 15.0f;
}

bool AChar_TribalWarrior::IsExhausted() const
{
    return TribalStats.Stamina < 10.0f;
}

float AChar_TribalWarrior::GetCombatEffectiveness() const
{
    float BaseEffectiveness = TribalStats.CombatExperience / 100.0f;
    
    // Reduce effectiveness based on health, hunger, thirst, and stamina
    float HealthModifier = TribalStats.Health / 100.0f;
    float HungerModifier = FMath::Max(0.5f, TribalStats.Hunger / 100.0f);
    float ThirstModifier = FMath::Max(0.5f, TribalStats.Thirst / 100.0f);
    float StaminaModifier = FMath::Max(0.3f, TribalStats.Stamina / 100.0f);
    
    return BaseEffectiveness * HealthModifier * HungerModifier * ThirstModifier * StaminaModifier;
}

void AChar_TribalWarrior::UpdateCharacterAppearance()
{
    if (!GetMesh())
        return;
    
    // Apply skin material
    if (SkinMaterial)
    {
        GetMesh()->SetMaterial(0, SkinMaterial);
    }
    
    // Apply clothing material
    if (ClothingMaterial)
    {
        GetMesh()->SetMaterial(1, ClothingMaterial);
    }
    
    // Apply tattoo materials to additional material slots
    for (int32 i = 0; i < TattooMaterials.Num() && i < 3; ++i)
    {
        if (TattooMaterials[i])
        {
            GetMesh()->SetMaterial(2 + i, TattooMaterials[i]);
        }
    }
}

void AChar_TribalWarrior::HandleStateTransitions()
{
    // Automatic state transitions based on survival stats
    if (!IsAlive())
    {
        SetWarriorState(EChar_WarriorState::Wounded);
        return;
    }
    
    if (IsWounded() && CurrentState != EChar_WarriorState::Wounded)
    {
        SetWarriorState(EChar_WarriorState::Wounded);
        return;
    }
    
    if (IsExhausted() && CurrentState != EChar_WarriorState::Resting)
    {
        SetWarriorState(EChar_WarriorState::Resting);
        return;
    }
    
    // Return to idle if conditions improve
    if (CurrentState == EChar_WarriorState::Wounded && TribalStats.Health > 50.0f)
    {
        SetWarriorState(EChar_WarriorState::Idle);
    }
    else if (CurrentState == EChar_WarriorState::Resting && TribalStats.Stamina > 80.0f)
    {
        SetWarriorState(EChar_WarriorState::Idle);
    }
}

void AChar_TribalWarrior::ApplyWeaponMesh(EChar_WeaponType WeaponType)
{
    if (!WeaponMesh)
        return;
    
    // Clear current weapon mesh
    WeaponMesh->SetStaticMesh(nullptr);
    WeaponMesh->SetVisibility(false);
    
    // TODO: Load appropriate weapon meshes based on weapon type
    // For now, just show/hide the component
    switch (WeaponType)
    {
        case EChar_WeaponType::StoneSpear:
        case EChar_WeaponType::BoneClub:
        case EChar_WeaponType::StoneAxe:
        case EChar_WeaponType::Sling:
        case EChar_WeaponType::BoneKnife:
            WeaponMesh->SetVisibility(true);
            break;
        case EChar_WeaponType::None:
        default:
            WeaponMesh->SetVisibility(false);
            break;
    }
}

void AChar_TribalWarrior::UpdateAnimationState()
{
    // TODO: Implement animation state updates based on CurrentState and EquippedWeapon
    // This would integrate with the Animation Blueprint system
    
    // For now, just log state changes for debugging
    static EChar_WarriorState LastLoggedState = EChar_WarriorState::Idle;
    if (CurrentState != LastLoggedState)
    {
        LastLoggedState = CurrentState;
        
        if (GEngine && GEngine->GetWorld() && GEngine->GetWorld()->GetTimeSeconds() > 5.0f)
        {
            GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::White, 
                FString::Printf(TEXT("%s animation state: %s"), 
                *GetName(), 
                *UEnum::GetValueAsString(CurrentState)));
        }
    }
}