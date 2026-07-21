#include "Char_TribalElder.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"

AChar_TribalElder::AChar_TribalElder()
{
    PrimaryActorTick.bCanEverTick = true;

    // Set elder character properties (slower movement, larger presence)
    GetCapsuleComponent()->SetCapsuleHalfHeight(92.0f);
    GetCapsuleComponent()->SetCapsuleRadius(38.0f);

    // Configure elder movement (slower, more deliberate)
    GetCharacterMovement()->MaxWalkSpeed = 300.0f;
    GetCharacterMovement()->JumpZVelocity = 300.0f;
    GetCharacterMovement()->AirControl = 0.1f;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 360.0f, 0.0f);

    // Create ceremonial item components
    StaffMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaffMesh"));
    StaffMesh->SetupAttachment(GetMesh(), TEXT("hand_r"));
    StaffMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
    StaffMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));

    FeatherHeaddressMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FeatherHeaddressMesh"));
    FeatherHeaddressMesh->SetupAttachment(GetMesh(), TEXT("head"));
    FeatherHeaddressMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 15.0f));

    BoneCharmsMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoneCharmsMesh"));
    BoneCharmsMesh->SetupAttachment(GetMesh(), TEXT("spine_02"));
    BoneCharmsMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));

    // Initialize elder wisdom stats
    KnowledgeLevel = 95.0f;
    DinosaurLore = 90.0f;
    SurvivalWisdom = 100.0f;
    YearsOfExperience = 45;

    // Set default assets to null - will be set up in BeginPlay
    AgedSkinMaterial = nullptr;
    CeremonialRobesMaterial = nullptr;
    WoodenStaffAsset = nullptr;
    FeatherHeaddressAsset = nullptr;
    BoneCharmsAsset = nullptr;
}

void AChar_TribalElder::BeginPlay()
{
    Super::BeginPlay();
    
    SetupElderAppearance();
    AttachCeremonialItems();
}

void AChar_TribalElder::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Elder could have special behaviors like periodic wisdom sharing
}

void AChar_TribalElder::SetupElderAppearance()
{
    if (USkeletalMeshComponent* MeshComp = GetMesh())
    {
        // Apply aged skin material
        if (AgedSkinMaterial)
        {
            MeshComp->SetMaterial(0, AgedSkinMaterial);
        }
        
        // Apply ceremonial robes material
        if (CeremonialRobesMaterial)
        {
            MeshComp->SetMaterial(1, CeremonialRobesMaterial);
        }
        
        // Set mesh positioning for elder
        MeshComp->SetRelativeLocation(FVector(0.0f, 0.0f, -92.0f));
        MeshComp->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
    }
}

void AChar_TribalElder::AttachCeremonialItems()
{
    // Attach wooden staff
    if (StaffMesh && WoodenStaffAsset)
    {
        StaffMesh->SetStaticMesh(WoodenStaffAsset);
        StaffMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    
    // Attach feather headdress
    if (FeatherHeaddressMesh && FeatherHeaddressAsset)
    {
        FeatherHeaddressMesh->SetStaticMesh(FeatherHeaddressAsset);
        FeatherHeaddressMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    
    // Attach bone charms
    if (BoneCharmsMesh && BoneCharmsAsset)
    {
        BoneCharmsMesh->SetStaticMesh(BoneCharmsAsset);
        BoneCharmsMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
}

void AChar_TribalElder::SetWisdomStats(float Knowledge, float DinoLore, float Wisdom, int32 Years)
{
    KnowledgeLevel = FMath::Clamp(Knowledge, 0.0f, 100.0f);
    DinosaurLore = FMath::Clamp(DinoLore, 0.0f, 100.0f);
    SurvivalWisdom = FMath::Clamp(Wisdom, 0.0f, 100.0f);
    YearsOfExperience = FMath::Max(Years, 0);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, 
            FString::Printf(TEXT("Elder Wisdom - Knowledge: %.1f, Dino Lore: %.1f, Survival: %.1f, Years: %d"), 
                KnowledgeLevel, DinosaurLore, SurvivalWisdom, YearsOfExperience));
    }
}

FString AChar_TribalElder::GetWisdomQuote()
{
    TArray<FString> WisdomQuotes = {
        TEXT("The great lizards teach us patience - they have ruled this land for countless seasons."),
        TEXT("Listen to the wind through the ferns - it carries the voices of those who came before."),
        TEXT("A sharp spear is worthless without the wisdom to know when not to use it."),
        TEXT("The earth remembers every footstep - tread carefully, young hunter."),
        TEXT("In the shadow of giants, we learn our true size and our true strength."),
        TEXT("Fire keeps the darkness at bay, but knowledge lights the path forward."),
        TEXT("The pack-hunters move as one mind - there is wisdom in unity."),
        TEXT("Even the mightiest thunder-foot was once small and vulnerable.")
    };
    
    int32 RandomIndex = FMath::RandRange(0, WisdomQuotes.Num() - 1);
    return WisdomQuotes[RandomIndex];
}

bool AChar_TribalElder::CanShareKnowledge() const
{
    return KnowledgeLevel >= 50.0f && YearsOfExperience >= 20;
}