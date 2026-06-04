#include "Char_VisualCustomization.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Engine.h"

UChar_VisualCustomization::UChar_VisualCustomization()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Update every second
    
    // Initialize default appearance
    AppearanceData = FChar_AppearanceData();
    bAppearanceApplied = false;
    LastUpdateTime = 0.0f;
}

void UChar_VisualCustomization::BeginPlay()
{
    Super::BeginPlay();
    
    // Apply initial appearance after a short delay to ensure mesh is loaded
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UChar_VisualCustomization::ApplyAppearance, 0.5f, false);
}

void UChar_VisualCustomization::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Periodic check to ensure appearance is maintained
    LastUpdateTime += DeltaTime;
    if (LastUpdateTime >= 5.0f && !bAppearanceApplied)
    {
        ApplyAppearance();
        LastUpdateTime = 0.0f;
    }
}

void UChar_VisualCustomization::ApplyAppearance()
{
    USkeletalMeshComponent* MeshComp = GetCharacterMesh();
    if (!MeshComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("UChar_VisualCustomization: No mesh component found"));
        return;
    }

    // Update mesh based on body type
    UpdateMeshBasedOnBodyType();
    
    // Update materials based on appearance data
    UpdateMaterialsBasedOnAppearance();
    
    // Apply material parameters
    ApplyMaterialParameters();
    
    bAppearanceApplied = true;
    
    UE_LOG(LogTemp, Log, TEXT("UChar_VisualCustomization: Appearance applied successfully"));
}

void UChar_VisualCustomization::RandomizeAppearance()
{
    // Randomize body type
    int32 BodyTypeIndex = FMath::RandRange(0, 4);
    AppearanceData.BodyType = static_cast<EChar_BodyType>(BodyTypeIndex);
    
    // Randomize skin tone
    int32 SkinToneIndex = FMath::RandRange(0, 4);
    AppearanceData.SkinTone = static_cast<EChar_SkinTone>(SkinToneIndex);
    
    // Randomize clothing style
    int32 ClothingIndex = FMath::RandRange(0, 4);
    AppearanceData.ClothingStyle = static_cast<EChar_ClothingStyle>(ClothingIndex);
    
    // Randomize colors
    AppearanceData.SkinColor = FLinearColor(
        FMath::RandRange(0.3f, 0.9f),
        FMath::RandRange(0.2f, 0.7f),
        FMath::RandRange(0.1f, 0.5f),
        1.0f
    );
    
    AppearanceData.ClothingColor = FLinearColor(
        FMath::RandRange(0.2f, 0.6f),
        FMath::RandRange(0.1f, 0.5f),
        FMath::RandRange(0.05f, 0.3f),
        1.0f
    );
    
    // Randomize features
    AppearanceData.bHasWarPaint = FMath::RandBool();
    AppearanceData.bHasScars = FMath::RandBool();
    AppearanceData.MuscleDefinition = FMath::RandRange(0.2f, 1.0f);
    
    // Apply the randomized appearance
    ApplyAppearance();
    
    UE_LOG(LogTemp, Log, TEXT("UChar_VisualCustomization: Appearance randomized"));
}

void UChar_VisualCustomization::SetBodyType(EChar_BodyType NewBodyType)
{
    AppearanceData.BodyType = NewBodyType;
    UpdateMeshBasedOnBodyType();
    ApplyMaterialParameters();
}

void UChar_VisualCustomization::SetSkinTone(EChar_SkinTone NewSkinTone)
{
    AppearanceData.SkinTone = NewSkinTone;
    
    // Update skin color based on tone
    switch (NewSkinTone)
    {
        case EChar_SkinTone::Light:
            AppearanceData.SkinColor = FLinearColor(0.9f, 0.8f, 0.7f, 1.0f);
            break;
        case EChar_SkinTone::Medium:
            AppearanceData.SkinColor = FLinearColor(0.7f, 0.6f, 0.4f, 1.0f);
            break;
        case EChar_SkinTone::Dark:
            AppearanceData.SkinColor = FLinearColor(0.4f, 0.3f, 0.2f, 1.0f);
            break;
        case EChar_SkinTone::Tanned:
            AppearanceData.SkinColor = FLinearColor(0.8f, 0.5f, 0.3f, 1.0f);
            break;
        case EChar_SkinTone::Weathered:
            AppearanceData.SkinColor = FLinearColor(0.6f, 0.4f, 0.3f, 1.0f);
            break;
    }
    
    ApplyMaterialParameters();
}

void UChar_VisualCustomization::SetClothingStyle(EChar_ClothingStyle NewStyle)
{
    AppearanceData.ClothingStyle = NewStyle;
    
    // Update clothing color based on style
    switch (NewStyle)
    {
        case EChar_ClothingStyle::AnimalHide:
            AppearanceData.ClothingColor = FLinearColor(0.4f, 0.3f, 0.2f, 1.0f);
            break;
        case EChar_ClothingStyle::PlantFiber:
            AppearanceData.ClothingColor = FLinearColor(0.3f, 0.4f, 0.2f, 1.0f);
            break;
        case EChar_ClothingStyle::BoneOrnaments:
            AppearanceData.ClothingColor = FLinearColor(0.8f, 0.8f, 0.7f, 1.0f);
            break;
        case EChar_ClothingStyle::FeatheredGarb:
            AppearanceData.ClothingColor = FLinearColor(0.6f, 0.4f, 0.2f, 1.0f);
            break;
        case EChar_ClothingStyle::MinimalWrap:
            AppearanceData.ClothingColor = FLinearColor(0.5f, 0.3f, 0.2f, 1.0f);
            break;
    }
    
    UpdateMaterialsBasedOnAppearance();
    ApplyMaterialParameters();
}

void UChar_VisualCustomization::SetWarPaint(bool bEnabled)
{
    AppearanceData.bHasWarPaint = bEnabled;
    ApplyMaterialParameters();
}

void UChar_VisualCustomization::SetScars(bool bEnabled)
{
    AppearanceData.bHasScars = bEnabled;
    ApplyMaterialParameters();
}

void UChar_VisualCustomization::UpdateMeshBasedOnBodyType()
{
    USkeletalMeshComponent* MeshComp = GetCharacterMesh();
    if (!MeshComp)
    {
        return;
    }

    // Try to load body type specific mesh if available
    if (BodyTypeMeshes.IsValidIndex(static_cast<int32>(AppearanceData.BodyType)))
    {
        TSoftObjectPtr<USkeletalMesh> MeshPtr = BodyTypeMeshes[static_cast<int32>(AppearanceData.BodyType)];
        if (USkeletalMesh* Mesh = MeshPtr.LoadSynchronous())
        {
            MeshComp->SetSkeletalMesh(Mesh);
            UE_LOG(LogTemp, Log, TEXT("UChar_VisualCustomization: Updated mesh for body type"));
        }
    }
    
    // Adjust scale based on body type
    FVector Scale = FVector(1.0f);
    switch (AppearanceData.BodyType)
    {
        case EChar_BodyType::Lean:
            Scale = FVector(0.95f, 0.95f, 1.02f);
            break;
        case EChar_BodyType::Muscular:
            Scale = FVector(1.05f, 1.05f, 1.0f);
            break;
        case EChar_BodyType::Stocky:
            Scale = FVector(1.1f, 1.1f, 0.98f);
            break;
        case EChar_BodyType::Tall:
            Scale = FVector(0.98f, 0.98f, 1.08f);
            break;
        case EChar_BodyType::Elder:
            Scale = FVector(0.96f, 0.96f, 0.95f);
            break;
    }
    
    MeshComp->SetWorldScale3D(Scale);
}

void UChar_VisualCustomization::UpdateMaterialsBasedOnAppearance()
{
    USkeletalMeshComponent* MeshComp = GetCharacterMesh();
    if (!MeshComp)
    {
        return;
    }

    // Apply skin material if available
    if (SkinMaterials.IsValidIndex(static_cast<int32>(AppearanceData.SkinTone)))
    {
        TSoftObjectPtr<UMaterialInterface> MaterialPtr = SkinMaterials[static_cast<int32>(AppearanceData.SkinTone)];
        if (UMaterialInterface* Material = MaterialPtr.LoadSynchronous())
        {
            MeshComp->SetMaterial(0, Material); // Assuming skin is material slot 0
        }
    }
    
    // Apply clothing material if available
    if (ClothingMaterials.IsValidIndex(static_cast<int32>(AppearanceData.ClothingStyle)))
    {
        TSoftObjectPtr<UMaterialInterface> MaterialPtr = ClothingMaterials[static_cast<int32>(AppearanceData.ClothingStyle)];
        if (UMaterialInterface* Material = MaterialPtr.LoadSynchronous())
        {
            MeshComp->SetMaterial(1, Material); // Assuming clothing is material slot 1
        }
    }
}

USkeletalMeshComponent* UChar_VisualCustomization::GetCharacterMesh() const
{
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        return Character->GetMesh();
    }
    
    return GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
}

void UChar_VisualCustomization::ApplyMaterialParameters()
{
    USkeletalMeshComponent* MeshComp = GetCharacterMesh();
    if (!MeshComp)
    {
        return;
    }

    // Create dynamic material instances and set parameters
    for (int32 MaterialIndex = 0; MaterialIndex < MeshComp->GetNumMaterials(); ++MaterialIndex)
    {
        UMaterialInterface* BaseMaterial = MeshComp->GetMaterial(MaterialIndex);
        if (BaseMaterial)
        {
            UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
            if (DynamicMaterial)
            {
                // Set color parameters
                DynamicMaterial->SetVectorParameterValue(TEXT("SkinColor"), AppearanceData.SkinColor);
                DynamicMaterial->SetVectorParameterValue(TEXT("ClothingColor"), AppearanceData.ClothingColor);
                
                // Set scalar parameters
                DynamicMaterial->SetScalarParameterValue(TEXT("MuscleDefinition"), AppearanceData.MuscleDefinition);
                DynamicMaterial->SetScalarParameterValue(TEXT("HasWarPaint"), AppearanceData.bHasWarPaint ? 1.0f : 0.0f);
                DynamicMaterial->SetScalarParameterValue(TEXT("HasScars"), AppearanceData.bHasScars ? 1.0f : 0.0f);
                
                MeshComp->SetMaterial(MaterialIndex, DynamicMaterial);
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("UChar_VisualCustomization: Material parameters applied"));
}