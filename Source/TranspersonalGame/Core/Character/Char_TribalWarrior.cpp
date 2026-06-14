#include "Char_TribalWarrior.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"

AChar_TribalWarrior::AChar_TribalWarrior()
{
    PrimaryActorTick.bCanEverTick = false;

    // Set default character mesh to UE5 mannequin
    static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshAsset(TEXT("/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple"));
    if (MeshAsset.Succeeded())
    {
        GetMesh()->SetSkeletalMesh(MeshAsset.Object);
        GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));
        GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
    }

    // Initialize default appearance
    AppearanceData = FChar_TribalAppearance();

    // Setup default equipment slots
    EquipmentSlots.SetNum(6);
    
    // Main weapon (right hand)
    EquipmentSlots[0].SocketName = TEXT("hand_rSocket");
    EquipmentSlots[0].RelativeTransform = FTransform(FRotator(0, 0, 0), FVector(0, 0, 0), FVector(1, 1, 1));
    
    // Secondary weapon (back)
    EquipmentSlots[1].SocketName = TEXT("spine_03Socket");
    EquipmentSlots[1].RelativeTransform = FTransform(FRotator(0, 0, 180), FVector(-10, 0, 0), FVector(1, 1, 1));
    
    // Shield (left hand)
    EquipmentSlots[2].SocketName = TEXT("hand_lSocket");
    EquipmentSlots[2].RelativeTransform = FTransform(FRotator(0, 90, 0), FVector(0, 0, 0), FVector(1, 1, 1));
    
    // Belt pouch
    EquipmentSlots[3].SocketName = TEXT("pelvisSocket");
    EquipmentSlots[3].RelativeTransform = FTransform(FRotator(0, 0, 0), FVector(0, 15, -5), FVector(1, 1, 1));
    
    // Necklace/amulet
    EquipmentSlots[4].SocketName = TEXT("neckSocket");
    EquipmentSlots[4].RelativeTransform = FTransform(FRotator(0, 0, 0), FVector(0, 0, 0), FVector(1, 1, 1));
    
    // Head ornament
    EquipmentSlots[5].SocketName = TEXT("headSocket");
    EquipmentSlots[5].RelativeTransform = FTransform(FRotator(0, 0, 0), FVector(0, 0, 5), FVector(1, 1, 1));

    // Initialize character stats with some variation
    Strength = FMath::RandRange(40.0f, 80.0f);
    Endurance = FMath::RandRange(45.0f, 75.0f);
    Agility = FMath::RandRange(35.0f, 70.0f);
    Survival = FMath::RandRange(50.0f, 90.0f);
}

void AChar_TribalWarrior::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeEquipmentComponents();
    ApplyAppearanceToMesh();
}

void AChar_TribalWarrior::SetAppearanceData(const FChar_TribalAppearance& NewAppearance)
{
    AppearanceData = NewAppearance;
    ApplyAppearanceToMesh();
}

void AChar_TribalWarrior::RandomizeAppearance()
{
    AppearanceData = FChar_TribalAppearance(); // Constructor randomizes values
    ApplyAppearanceToMesh();
}

void AChar_TribalWarrior::ApplyAppearanceToMesh()
{
    if (!GetMesh() || !GetMesh()->GetSkeletalMeshAsset())
    {
        return;
    }

    // Create dynamic material instances for customization
    TArray<UMaterialInterface*> Materials = GetMesh()->GetMaterials();
    
    for (int32 i = 0; i < Materials.Num(); i++)
    {
        if (Materials[i])
        {
            UMaterialInstanceDynamic* DynamicMat = UMaterialInstanceDynamic::Create(Materials[i], this);
            if (DynamicMat)
            {
                // Apply skin tone variation
                DynamicMat->SetScalarParameterValue(TEXT("SkinTone"), AppearanceData.SkinTone);
                
                // Apply body build variation (affects muscle definition)
                DynamicMat->SetScalarParameterValue(TEXT("BodyBuild"), AppearanceData.BodyBuild);
                
                // Apply tribal markings
                DynamicMat->SetScalarParameterValue(TEXT("TribalMarkings"), AppearanceData.TribalMarkings);
                
                GetMesh()->SetMaterial(i, DynamicMat);
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Applied appearance: SkinTone=%.2f, BodyBuild=%.2f, Clothing=%d, Weapons=%d"), 
           AppearanceData.SkinTone, AppearanceData.BodyBuild, AppearanceData.ClothingStyle, AppearanceData.WeaponSet);
}

void AChar_TribalWarrior::AttachEquipment(int32 SlotIndex, UStaticMesh* EquipmentMesh)
{
    if (!IsValidEquipmentSlot(SlotIndex) || !EquipmentMesh)
    {
        return;
    }

    if (EquipmentComponents.IsValidIndex(SlotIndex) && EquipmentComponents[SlotIndex])
    {
        EquipmentComponents[SlotIndex]->SetStaticMesh(EquipmentMesh);
        EquipmentComponents[SlotIndex]->SetVisibility(EquipmentSlots[SlotIndex].bIsVisible);
        
        UE_LOG(LogTemp, Log, TEXT("Attached equipment to slot %d: %s"), SlotIndex, *EquipmentMesh->GetName());
    }
}

void AChar_TribalWarrior::DetachEquipment(int32 SlotIndex)
{
    if (!IsValidEquipmentSlot(SlotIndex))
    {
        return;
    }

    if (EquipmentComponents.IsValidIndex(SlotIndex) && EquipmentComponents[SlotIndex])
    {
        EquipmentComponents[SlotIndex]->SetStaticMesh(nullptr);
        EquipmentComponents[SlotIndex]->SetVisibility(false);
    }
}

void AChar_TribalWarrior::SetEquipmentVisibility(int32 SlotIndex, bool bVisible)
{
    if (!IsValidEquipmentSlot(SlotIndex))
    {
        return;
    }

    EquipmentSlots[SlotIndex].bIsVisible = bVisible;
    
    if (EquipmentComponents.IsValidIndex(SlotIndex) && EquipmentComponents[SlotIndex])
    {
        EquipmentComponents[SlotIndex]->SetVisibility(bVisible);
    }
}

void AChar_TribalWarrior::SetCharacterStats(float NewStrength, float NewEndurance, float NewAgility, float NewSurvival)
{
    Strength = FMath::Clamp(NewStrength, 0.0f, 100.0f);
    Endurance = FMath::Clamp(NewEndurance, 0.0f, 100.0f);
    Agility = FMath::Clamp(NewAgility, 0.0f, 100.0f);
    Survival = FMath::Clamp(NewSurvival, 0.0f, 100.0f);
}

float AChar_TribalWarrior::GetOverallFitness() const
{
    return (Strength + Endurance + Agility + Survival) / 4.0f;
}

void AChar_TribalWarrior::SetupDefaultAppearance()
{
    // Setup a balanced default appearance
    AppearanceData.SkinTone = 0.6f;
    AppearanceData.BodyBuild = 0.7f;
    AppearanceData.ClothingStyle = 1;
    AppearanceData.WeaponSet = 0;
    AppearanceData.TribalMarkings = 1;
    
    ApplyAppearanceToMesh();
    
    UE_LOG(LogTemp, Warning, TEXT("Applied default tribal warrior appearance"));
}

void AChar_TribalWarrior::SetupDefaultEquipment()
{
    // This would load default equipment meshes
    // For now, just log the setup
    UE_LOG(LogTemp, Warning, TEXT("Setting up default equipment for tribal warrior"));
    
    // Equipment setup would happen here when we have actual mesh assets
    ValidateEquipmentSlots();
}

void AChar_TribalWarrior::GenerateRandomVariant()
{
    RandomizeAppearance();
    
    // Randomize stats as well
    Strength = FMath::RandRange(30.0f, 90.0f);
    Endurance = FMath::RandRange(40.0f, 85.0f);
    Agility = FMath::RandRange(25.0f, 80.0f);
    Survival = FMath::RandRange(45.0f, 95.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("Generated random tribal warrior variant - Fitness: %.1f"), GetOverallFitness());
}

void AChar_TribalWarrior::InitializeEquipmentComponents()
{
    EquipmentComponents.Empty();
    EquipmentComponents.SetNum(EquipmentSlots.Num());
    
    for (int32 i = 0; i < EquipmentSlots.Num(); i++)
    {
        UStaticMeshComponent* EquipComp = CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("Equipment_%d"), i));
        if (EquipComp)
        {
            EquipComp->SetupAttachment(GetMesh(), EquipmentSlots[i].SocketName);
            EquipComp->SetRelativeTransform(EquipmentSlots[i].RelativeTransform);
            EquipComp->SetVisibility(EquipmentSlots[i].bIsVisible);
            EquipComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            
            EquipmentComponents[i] = EquipComp;
        }
    }
}

void AChar_TribalWarrior::UpdateMaterialParameters()
{
    // Update material parameters based on current appearance data
    ApplyAppearanceToMesh();
}

void AChar_TribalWarrior::ValidateEquipmentSlots()
{
    for (int32 i = 0; i < EquipmentSlots.Num(); i++)
    {
        if (EquipmentSlots[i].SocketName == NAME_None)
        {
            UE_LOG(LogTemp, Warning, TEXT("Equipment slot %d has invalid socket name"), i);
        }
    }
}

bool AChar_TribalWarrior::IsValidEquipmentSlot(int32 SlotIndex) const
{
    return EquipmentSlots.IsValidIndex(SlotIndex);
}