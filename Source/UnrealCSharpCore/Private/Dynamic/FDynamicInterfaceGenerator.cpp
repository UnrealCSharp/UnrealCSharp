#include "Dynamic/FDynamicInterfaceGenerator.h"
#include "Bridge/FTypeBridge.h"
#include "Common/FUnrealCSharpFunctionLibrary.h"
#include "CoreMacro/Macro.h"
#include "CoreMacro/ClassMacro.h"
#include "Domain/FMonoDomain.h"
#include "Dynamic/FDynamicGeneratorCore.h"
#if WITH_EDITOR
#include "BlueprintActionDatabase.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Dynamic/FDynamicBlueprintExtensionScope.h"
#endif
#include "UEVersion.h"

TMap<UClass*, FString> FDynamicInterfaceGenerator::NamespaceMap;

TMap<FString, UClass*> FDynamicInterfaceGenerator::DynamicInterfaceMap;

TSet<UClass*> FDynamicInterfaceGenerator::DynamicInterfaceSet;

void FDynamicInterfaceGenerator::Generator()
{
	FDynamicGeneratorCore::Generator(CLASS_U_INTERFACE_ATTRIBUTE,
	                                 [](MonoClass* InMonoClass)
	                                 {
		                                 if (InMonoClass == nullptr)
		                                 {
			                                 return;
		                                 }

		                                 if (!FMonoDomain::Type_Is_Class(FMonoDomain::Class_Get_Type(InMonoClass)))
		                                 {
			                                 return;
		                                 }

		                                 const auto ClassName = FString(FMonoDomain::Class_Get_Name(InMonoClass));

		                                 auto Node = FDynamicDependencyGraph::FNode(ClassName, [InMonoClass]()
		                                 {
			                                 Generator(InMonoClass);
		                                 });

		                                 if (const auto ParentMonoClass = FMonoDomain::Class_Get_Parent(InMonoClass))
		                                 {
			                                 if (FDynamicGeneratorCore::ClassHasAttr(
				                                 ParentMonoClass, CLASS_U_INTERFACE_ATTRIBUTE))
			                                 {
				                                 const auto ParentClassName = FString(
					                                 FMonoDomain::Class_Get_Name(ParentMonoClass));

				                                 Node.Dependency(FDynamicDependencyGraph::FDependency{
					                                 ParentClassName, false
				                                 });
			                                 }
		                                 }

		                                 FDynamicGeneratorCore::GeneratorFunction(InMonoClass, Node);

		                                 FDynamicGeneratorCore::AddNode(Node);
	                                 });
}

#if WITH_EDITOR
void FDynamicInterfaceGenerator::CodeAnalysisGenerator()
{
	FDynamicGeneratorCore::CodeAnalysisGenerator(DYNAMIC_INTERFACE,
	                                             [](const FString& InNameSpace, const FString& InName)
	                                             {
		                                             if (!DynamicInterfaceMap.Contains(InName))
		                                             {
			                                             GeneratorInterface(FDynamicGeneratorCore::GetOuter(),
			                                                                InNameSpace,
			                                                                InName,
			                                                                UInterface::StaticClass());
		                                             }
	                                             });
}

bool FDynamicInterfaceGenerator::IsDynamicInterface(MonoClass* InMonoClass)
{
	return FDynamicGeneratorCore::IsDynamic(InMonoClass, CLASS_U_INTERFACE_ATTRIBUTE);
}
#endif

void FDynamicInterfaceGenerator::Generator(MonoClass* InMonoClass)
{
	if (InMonoClass == nullptr)
	{
		return;
	}

	if (!FMonoDomain::Type_Is_Class(FMonoDomain::Class_Get_Type(InMonoClass)))
	{
		return;
	}

	const auto ClassName = FString(FMonoDomain::Class_Get_Name(InMonoClass));

	const auto ClassNamespace = FString(FMonoDomain::Class_Get_Namespace(InMonoClass));

	const auto Outer = FDynamicGeneratorCore::GetOuter();

	const auto ParentMonoClass = FMonoDomain::Class_Get_Parent(InMonoClass);

	const auto ParentMonoType = FMonoDomain::Class_Get_Type(ParentMonoClass);

	const auto ParentMonoReflectionType = FMonoDomain::Type_Get_Object(ParentMonoType);

	const auto ParentPathName = FTypeBridge::GetPathName(ParentMonoReflectionType);

	const auto ParentClass = LoadClass<UInterface>(nullptr, *ParentPathName);

#if WITH_EDITOR
	const UClass* OldClass{};
#endif

	UClass* Class{};

	if (DynamicInterfaceMap.Contains(ClassName))
	{
		Class = DynamicInterfaceMap[ClassName];

		Class->PurgeClass(true);

		GeneratorInterface(ClassNamespace, ClassName, Class, ParentClass,
		                   [InMonoClass](UClass* InInterface)
		                   {
			                   ProcessGenerator(InMonoClass, InInterface);
		                   });

#if WITH_EDITOR
		OldClass = Class;
#endif
	}
	else
	{
		Class = GeneratorInterface(Outer, ClassNamespace, ClassName, ParentClass,
		                           [InMonoClass](UClass* InInterface)
		                           {
			                           ProcessGenerator(InMonoClass, InInterface);
		                           });
	}

#if WITH_EDITOR
	if (OldClass != nullptr)
	{
		ReInstance(Class);
	}
#endif

	FDynamicGeneratorCore::Completed(ClassName);
}

bool FDynamicInterfaceGenerator::IsDynamicInterface(const UClass* InClass)
{
	return DynamicInterfaceSet.Contains(InClass);
}

FString FDynamicInterfaceGenerator::GetNameSpace(const UClass* InClass)
{
	const auto FoundNameSpace = NamespaceMap.Find(InClass);

	return FoundNameSpace != nullptr ? *FoundNameSpace : FString{};
}

void FDynamicInterfaceGenerator::BeginGenerator(UClass* InClass, UClass* InParentClass)
{
	InClass->PropertyLink = InParentClass->PropertyLink;

	InClass->ClassWithin = InParentClass->ClassWithin;

	InClass->ClassConfigName = InParentClass->ClassConfigName;

	InClass->SetSuperStruct(InParentClass);

#if UE_U_CLASS_ADD_REFERENCED_OBJECTS
	InClass->ClassAddReferencedObjects = InParentClass->ClassAddReferencedObjects;
#endif

	InClass->ClassCastFlags |= InParentClass->ClassCastFlags;

	InClass->ClassFlags |= CLASS_Abstract | CLASS_Native | CLASS_Interface;
}

void FDynamicInterfaceGenerator::ProcessGenerator(MonoClass* InMonoClass, UClass* InClass)
{
	FDynamicGeneratorCore::SetFlags(InClass, FMonoDomain::Custom_Attrs_From_Class(InMonoClass));

	GeneratorFunction(InMonoClass, InClass);
}

void FDynamicInterfaceGenerator::EndGenerator(UClass* InClass)
{
	InClass->ClearInternalFlags(EInternalObjectFlags::Native);

	InClass->Bind();

	InClass->StaticLink(true);

	InClass->AssembleReferenceTokenStream();

	FUnrealCSharpFunctionLibrary::SetClassDefaultObject(InClass);

	(*InClass->ClassConstructor)(FObjectInitializer(InClass->GetDefaultObject(false),
	                                                InClass->GetSuperClass()->GetDefaultObject(),
	                                                EObjectInitializerOptions::None));

	InClass->SetInternalFlags(EInternalObjectFlags::Native);

#if WITH_EDITOR
	if (GEditor)
	{
		FBlueprintActionDatabase& ActionDatabase = FBlueprintActionDatabase::Get();

		ActionDatabase.ClearAssetActions(InClass);

		ActionDatabase.RefreshClassActions(InClass);
	}
#endif

#if UE_NOTIFY_REGISTRATION_EVENT
#if !WITH_EDITOR
	NotifyRegistrationEvent(*InClass->GetDefaultObject(false)->GetPackage()->GetName(),
							*InClass->GetDefaultObject(false)->GetName(),
							ENotifyRegistrationType::NRT_ClassCDO,
							ENotifyRegistrationPhase::NRP_Finished,
							nullptr,
							false,
							InClass->GetDefaultObject(false));


	NotifyRegistrationEvent(*InClass->GetPackage()->GetName(),
							*InClass->GetName(),
							ENotifyRegistrationType::NRT_Class,
							ENotifyRegistrationPhase::NRP_Finished,
							nullptr,
							false,
							InClass);
#endif
#endif
}

void FDynamicInterfaceGenerator::GeneratorInterface(const FString& InNameSpace, const FString& InName,
                                                    UClass* InClass, UClass* InParentClass,
                                                    const TFunction<void(UClass*)>& InProcessGenerator)
{
	NamespaceMap.Add(InClass, InNameSpace);

	DynamicInterfaceMap.Add(InName, InClass);

	DynamicInterfaceSet.Add(InClass);

	BeginGenerator(InClass, InParentClass);

	InProcessGenerator(InClass);

	EndGenerator(InClass);
}

UClass* FDynamicInterfaceGenerator::GeneratorInterface(UPackage* InOuter, const FString& InNameSpace,
                                                       const FString& InName, UClass* InParentClass)
{
	return GeneratorInterface(InOuter, InNameSpace, InName, InParentClass,
	                          [](UClass* InClass)
	                          {
	                          });
}

UClass* FDynamicInterfaceGenerator::GeneratorInterface(UPackage* InOuter, const FString& InNameSpace,
                                                       const FString& InName, UClass* InParentClass,
                                                       const TFunction<void(UClass*)>& InProcessGenerator)
{
	const auto Class = NewObject<UClass>(InOuter, *InName.RightChop(1), RF_Public);

	Class->AddToRoot();

	GeneratorInterface(InNameSpace, InName, Class, InParentClass, InProcessGenerator);

	return Class;
}

#if WITH_EDITOR
void FDynamicInterfaceGenerator::ReInstance(UClass* InClass)
{
	TArray<UBlueprintGeneratedClass*> BlueprintGeneratedClasses;

	FDynamicGeneratorCore::IteratorObject<UBlueprintGeneratedClass>(
		[InClass](const TObjectIterator<UBlueprintGeneratedClass>& InBlueprintGeneratedClass)
		{
			if (InBlueprintGeneratedClass->IsNative())
			{
				return false;
			}

			return InBlueprintGeneratedClass->ImplementsInterface(InClass);
		},
		[&BlueprintGeneratedClasses](const TObjectIterator<UBlueprintGeneratedClass>& InBlueprintGeneratedClass)
		{
			if (!FUnrealCSharpFunctionLibrary::IsSpecialClass(*InBlueprintGeneratedClass))
			{
				BlueprintGeneratedClasses.AddUnique(*InBlueprintGeneratedClass);
			}
		});


	for (const auto BlueprintGeneratedClass : BlueprintGeneratedClasses)
	{
		if (const auto Blueprint = Cast<UBlueprint>(BlueprintGeneratedClass->ClassGeneratedBy))
		{
			FDynamicBlueprintExtensionScope DynamicBlueprintExtensionScope(Blueprint);

			Blueprint->Modify();

			FBlueprintEditorUtils::RefreshAllNodes(Blueprint);

			FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

			constexpr auto BlueprintCompileOptions = EBlueprintCompileOptions::SkipGarbageCollection |
				EBlueprintCompileOptions::SkipSave;

			FKismetEditorUtilities::CompileBlueprint(Blueprint, BlueprintCompileOptions);
		}
	}
}
#endif

void FDynamicInterfaceGenerator::GeneratorFunction(MonoClass* InMonoClass, UClass* InClass)
{
	FDynamicGeneratorCore::GeneratorFunction(FDynamicGeneratorCore::UInterfaceToIInterface(InMonoClass),
	                                         InClass,
	                                         [](const UFunction* InFunction)
	                                         {
		                                         InFunction->SetInternalFlags(EInternalObjectFlags::Native);
	                                         });
}
